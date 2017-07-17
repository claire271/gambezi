#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#include <string.h>
#include <stdint.h>
#include "node.h"
#include "action.h"
#include "protocol_gambezi.h"

struct per_vhost_data_gambezi
{
	struct Node* root_node;
};

struct per_session_data_gambezi
{
	struct ActionQueue actions;
};

static int
callback_gambezi(struct lws *wsi,
                 enum lws_callback_reasons reason,
                 void *user,
                 void *in,
                 size_t len)
{
	// Websockets information
	struct per_session_data_gambezi *pss =
		(struct per_session_data_gambezi *) user;
	struct per_vhost_data_gambezi *vhd =
		(struct per_vhost_data_gambezi *) lws_protocol_vh_priv_get(lws_get_vhost(wsi),
		                                                           lws_get_protocol(wsi));

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
			                                  sizeof(struct per_vhost_data_gambezi));

			// Init data holders
			vhd->root_node = node_init("");
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Cleanup
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		{
			if (!vhd)
				break;
			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Initialize data related to a single connection
		case LWS_CALLBACK_ESTABLISHED:
			initActionQueue(&(pss->actions));
			break;

		////////////////////////////////////////////////////////////////////////////////
		// Writing back to the client
		case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			struct Action* action = useAction(&(pss->actions));

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
					lws_write(wsi, buffer, length, LWS_WRITE_BINARY);
					break;
				}
			}

			break;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Data from client
		case LWS_CALLBACK_RECEIVE:
		{
			const uint8_t* data = (const uint8_t*)in;
			switch(data[0])
			{
				// Client requested key ID
				case 0:
				{
					// Get the ID
					const uint8_t* parent_key = data + 1;
					const uint8_t* name = data + parent_key[0] + 2;
					uint8_t id = node_get_id(vhd->root_node,
					                         parent_key,
					                         name);

					// Queue and generate the response
					struct Action* action = addAction(&(pss->actions));
					action->type = PregeneratedRequest;
					uint8_t* buffer = action->action.pregeneratedRequest.buffer + LWS_PRE;
					int length = 0;
					// Packet header
					buffer[length++] = 0x00;
					// Key
					memcpy(buffer + length, parent_key, parent_key[0] + 1);
					(buffer + length)[0]++;
					(buffer + length)[(buffer + length)[0]] = id;
					length += parent_key[0] + 2;
					// Name
					memcpy(buffer + length, name, name[0] + 1);
					length += name[0] + 1;
					action->action.pregeneratedRequest.length = length;

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

static const struct lws_protocols protocols[] =
{
	{
		"gambezi-protocol",
		callback_gambezi,
		sizeof(struct per_session_data_gambezi),
		BUFFER_LENGTH, /* rx buf size must be >= permessage-deflate rx size */
	},
};

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

LWS_VISIBLE int
destroy_protocol_gambezi(struct lws_context *context)
{
	return 0;
}