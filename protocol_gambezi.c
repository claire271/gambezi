#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#include <stdint.h>

#include "limits.h"
#include "node.h"
#include "action.h"
#include "gambezi_generator.h"
#include "protocol_gambezi.h"
#include "subscription.h"

/**
 * Note: Most functions in this file are named this way because it is what
 * lwsws expects.
 */

/**
 * This function is called by lib_uv.
 * This callback is called for each connection (client).
 * Handles fixed rate key updates.
 */
static void
uv_timeout_cb_update(uv_timer_t *w
#if UV_VERSION_MAJOR == 0
		, int status
#endif
)
{
	// Get client session data
	struct session_data *psd = lws_container_of(w, struct session_data, timer);

	// Cycle through all subscriptions for this client
	for(int i = 0;i < MAX_SUBSCRIPTIONS;i++)
	{
		struct Subscription* subscription = &(psd->subscriptions[i]);
		if(subscription->node)
		{
			subscription->count++;
			if(subscription->count >= subscription->period)
			{
				// Queue up response
				struct Action* action = addAction(psd->actions);
				// No error
				if(action)
				{
					action->type = DataReturnRequest;
					action->action.dataReturnRequest.node = subscription->node;

					// Reset counter
					subscription->count = 0;
				}
				// Too many actions
				else
				{
					lwsl_notice("NOTICE: Too many actions.");
					// Not resetting counter to hopefully push this subscription
					// onto a more empty time slot
				}
			}
		}
	}

	// Request write data
	lws_callback_on_writable(psd->wsi);
}

/**
 * This function is called by libwebsockets.
 * This callback is called whenever this plugin needs to interact with lwsws.
 */
static int
callback_gambezi(struct lws *wsi,
                 enum lws_callback_reasons reason,
                 void *user,
                 void *in,
                 size_t len)
{
	// Websockets information
	struct session_data *psd = (struct session_data *) user;
	struct vhost_data *vhd = (struct vhost_data*)
		lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

	////////////////////////////////////////////////////////////////////////////////
	// Why is the callback called
	switch (reason)
	{
		////////////////////////////////////////////////////////////////////////////////
		// Entire protocol setup
		case LWS_CALLBACK_PROTOCOL_INIT:
		{
			// Init websockets stuff
			vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
			                                  lws_get_protocol(wsi),
			                                  sizeof(struct vhost_data));
			vhd->context = lws_get_context(wsi);

			// Init data holders
			uint8_t root_data[1];
			root_data[0] = 0;
			vhd->root_node = node_init(root_data, root_data, 0);
			// Handle error
			if(!vhd->root_node)
			{
				lwsl_err("ERROR: Unable to create root node");
				lwsl_err("SHUTDOWN REQUESTED");
				return -1;
			}
			vhd->root_node->key[0] = 0;
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Cleanup
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		{
			if (!vhd)
				break;
			node_free_all(vhd->root_node);
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Initialize data related to a single connection
		case LWS_CALLBACK_ESTABLISHED:
		{
			// Session init
			psd->wsi = wsi;

			// Create queue
			psd->actions = malloc(sizeof(struct ActionQueue));
			if(!psd->actions)
			{
				lwsl_err("ERROR: Unable to create action queue");
				lwsl_err("SHUTDOWN REQUESTED");
				return -1;
			}
			initActionQueue(psd->actions);

			// Create timer
			uv_timer_init(lws_uv_getloop(vhd->context, 0),
			              &(psd->timer));
			uv_timer_start(&(psd->timer),
			               uv_timeout_cb_update, DEFAULT_PERIOD, DEFAULT_PERIOD);
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Single connection destroyed
		case LWS_CALLBACK_CLOSED:
		{
			free(psd->actions);
			uv_timer_stop(&(psd->timer));
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Writing back to the client
		case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			struct Action* action = useAction(psd->actions);

			// Bail if there is no action
			if(!action) {
				break;
			}

			// Process queued responses
			switch(action->type)
			{
				case PregeneratedRequest:
				{
					uint8_t* buffer = action->action.pregeneratedRequest.buffer + LWS_PRE;
					int length = action->action.pregeneratedRequest.length;
					int written = lws_write(wsi, buffer, length, LWS_WRITE_BINARY);
					if(written < length)
					{
						lwsl_err("ERROR: Error writing to connection");
					}
					break;
				}
				case DataReturnRequest:
				{
					struct Node* node = action->action.dataReturnRequest.node;
					int written = lws_write(wsi, node->buffer, node->current_length, LWS_WRITE_BINARY);
					if(written < node->current_length)
					{
						lwsl_err("ERROR: Error writing to connection");
					}
					break;
				}
			}

			// Callback again just in case
			lws_callback_on_writable(wsi);
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Data from client
		case LWS_CALLBACK_RECEIVE:
		{
			uint8_t* data = (uint8_t*)in;
			switch(data[0])
			{
				////////////////////////////////////////////////////////////
				// Client requested key ID
				case PACKET_REQUEST_KEY:
				{
					// Extract data from the packet
					uint8_t* parent_key;
					uint8_t* name;
					readIDRequestPacket(data, &parent_key, &name);

					// Get node with matching name and parent key
					struct Node* node = get_node_with_id(vhd->root_node,
					                                     parent_key,
					                                     name);
					// No error
					if(node)
					{
						// Queue and generate the response
						struct Action* action = addAction(psd->actions);
						action->type = PregeneratedRequest;
						uint8_t* buffer = action->action.pregeneratedRequest.buffer + LWS_PRE;
						int length = writeIDResponsePacket(buffer, BUFFER_LENGTH, node);
						action->action.pregeneratedRequest.length = length;
					}
					// Error
					else
					{
						lwsl_err("ERROR: Unable to get node ID");
						//TODO: Send message to client about this condition
					}

					// Request callback to write to client
					lws_callback_on_writable(wsi);
					break;
				}
				////////////////////////////////////////////////////////////
				// Client set key value
				case PACKET_SET_KEY:
				{
					// Extract data from the packet
					uint8_t* key;
					uint16_t length;
					uint8_t* value;
					readValueSetPacket(data, &key, &length, &value);

					// Get node with matching key
					struct Node* node = node_traverse(vhd->root_node, key);

					// No error
					if(node)
					{
						// Set value and update subscribers
						node_set_value(node, value, length);
						node_notify_subscribers(node);
					}
					// Error
					else
					{
						lwsl_err("ERROR: Invalid node requested");
						//TODO: Send message to client about this condition
					}

					break;
				}
				////////////////////////////////////////////////////////////
				// Client set refresh rate
				case PACKET_SET_REFRESH_RATE:
				{
					uint16_t refresh_rate;
					readRefreshRateSetPacket(data, &refresh_rate);
					uv_timer_set_repeat(&(psd->timer), refresh_rate);
					break;
				}
				////////////////////////////////////////////////////////////
				// Client update subscription
				case PACKET_UPDATE_KEY_SUBSCRIPTION:
				{
					// Read packet
					uint8_t* key;
					uint8_t set_children;
					uint16_t refresh_skip;
					readSubscriptionUpdatePacket(data, &key, &set_children, &refresh_skip);
					struct Node* node = node_traverse(vhd->root_node, key);

					// No error
					if(node)
					{
						// Get updates as fast as possible
						if(refresh_skip == 0x0000)
						{
							int code = node_add_subscriber(node, psd);
							if(code < 0)
							{
								lwsl_err("ERROR: Unable to subscribe to node");
								//TODO: Send message to client about this condition
							}
						}
						// Unsubscribe
						else if(refresh_skip == 0xFFFF)
						{
							node_remove_subscriber(node, psd);
							struct Subscription* subscription = subscription_get_with_node(
								psd->subscriptions,
								node);
							if(subscription)
							{
								subscription->node = 0;
							}
						}
						// Update at fixed rate
						else
						{
							struct Subscription* subscription = subscription_get_with_node(
								psd->subscriptions,
								node);
							// Update existing subscription
							if(subscription)
							{
								if(subscription->period != refresh_skip)
								{
									subscription->period = refresh_skip;
									subscription->count = 0;
								}
							}
							// Create new subscription
							else
							{
								subscription = subscription_get_empty(psd->subscriptions);
								// No error
								if(subscription)
								{
									subscription->node = node;
									subscription->period = refresh_skip;
									subscription->count = 0;
								}
								// Error
								else
								{
									lwsl_err("ERROR: Unable to subscribe to node");
									//TODO: Send message to client about this condition
								}
							}
						}
					}
					// Error
					else
					{
						lwsl_err("ERROR: Invalid node requested");
						//TODO: Send message to client about this condition
					}

					break;
				}
				////////////////////////////////////////////////////////////
				// Client request node value
				case PACKET_REQUEST_KEY_VALUE:
				{
					uint8_t* key;
					uint8_t get_children;
					readValueRequestPacket(data, &key, &get_children);
					struct Node* node = node_traverse(vhd->root_node, key);

					// No error
					if(node)
					{
						// Queue and generate the response
						struct Action* action = addAction(psd->actions);
						action->type = DataReturnRequest;
						action->action.dataReturnRequest.node = node;
					}
					// Error
					else
					{
						lwsl_err("ERROR: Invalid node requested");
						//TODO: Send message to client about this condition
					}

					// Request callback to write to client
					lws_callback_on_writable(wsi);
					break;
				}
			}
			break;
		}
	}
	return 0;
}

/**
 * Struct describing this protocol
 */
static const struct lws_protocols protocols[] =
{
	{
		"gambezi-protocol",
		callback_gambezi,
		sizeof(struct session_data),
		BUFFER_LENGTH, /* rx buf size must be >= permessage-deflate rx size */
	},
};

/**
 * lwsws initialization of protocol
 * From example code
 */
LWS_VISIBLE int
init_protocol_gambezi(struct lws_context *context,
                      struct lws_plugin_capability *c)
{
	if (c->api_magic != LWS_PLUGIN_API_MAGIC)
	{
		lwsl_err("Plugin API %d, library API %d",
		         LWS_PLUGIN_API_MAGIC,
		         c->api_magic);
		return 1;
	}

	c->protocols = protocols;
	c->count_protocols = ARRAY_SIZE(protocols);
	c->extensions = NULL;
	c->count_extensions = 0;

	return 0;
}

/**
 * lwsws cleanup of protocol
 */
LWS_VISIBLE int
destroy_protocol_gambezi(struct lws_context *context)
{
	return 0;
}
