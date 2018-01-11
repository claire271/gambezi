#include <libwebsockets.h>
#include <string.h>
#include "arpa/inet.h"
#include "action.h"
#include "node.h"
#include "protocol_gambezi.h"
#include "gambezi_generator.h"

/**
 * Allocates a node with a given name and type, and returns a pointer to it
 */
struct Node* node_init(const uint8_t* name, const uint8_t* parent_key, uint8_t id)
{
	// Allocate node
	struct Node* node = malloc(sizeof(struct Node));
	// Unable to allocate
	if(!node)
	{
		return 0;
	}

	// Assign name
	memcpy(node->name, name, name[0] + 1);

	// Zero children nodes
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		node->children[i] = 0;
	}

	// Zero subscribers
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		node->subscribers[i] = 0;
	}

	// Allocate buffer
	int key_length = parent_key[0] + 2;
	int buffer_length = 1 + key_length + 2;
	node->buffer = malloc(LWS_PRE + buffer_length);
	// Unable to allocate
	if(!node->buffer)
	{
		return 0;
	}
	// Shift buffer
	node->buffer += LWS_PRE;

	// Set header
	node->buffer[0] = PACKET_RETURN_KEY_VALUE;

	// Set key
	node->key = node->buffer + 1;
	memcpy(node->key, parent_key, parent_key[0] + 1);
	node->key[0]++;
	node->key[node->key[0]] = id;

	// Set data
	*((uint16_t*)(node->key + key_length)) = htons(0);
	node->data = node->key + key_length + 2;

	// Set lengths
	node->current_length = buffer_length;
	node->allocated_length = buffer_length;

	// Success
	return node;
}

/**
 * Gets the node for a given key
 */
struct Node* node_traverse(struct Node* root_node, const uint8_t* key)
{
	int depth = key[0];
	for(;depth > 0;depth--)
	{
		// Descend a level
		key++;
		root_node = root_node->children[*key];

		// Invalid key traversal
		if(!root_node)
		{
			return 0;
		}
	}

	// Success
	return root_node;
}

/**
 * Gets the ID of a node, creates it if it does not exist yet
 */
struct Node* node_get_with_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name)
{
	// Traverse tree to immediate parent
	struct Node* parent_node = node_traverse(root_node, parent_key);
	// Invalid parent
	if(!parent_node)
	{
		return 0;
	}

	// Search for the node
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		// Node not found
		if(!(parent_node->children[i]))
		{
			parent_node->children[i] = node_init(name, parent_node->key, i);
			// Error creating node
			if(!parent_node->children[i])
			{
				return 0;
			}
			return parent_node->children[i];
		}

		// Node found
		if(!(memcmp(name, parent_node->children[i]->name, name[0] + 1)))
		{
			return parent_node->children[i];
		}
	}

	// No more room to add child node
	return 0;
}

/**
 * Sets the data of a given node
 */
int node_set_value(struct Node* node, const uint8_t* data, uint16_t data_length)
{
	// Calculate lengths
	int key_length = node->key[0] + 1;
	int buffer_length = 1 + key_length + 2 + data_length;

	// Check if current allocation is large enough
	if(buffer_length > node->allocated_length)
	{
		// Allocate buffer
		uint8_t* buffer = malloc(LWS_PRE + buffer_length) + LWS_PRE;
		// Unable to allocate
		if(!buffer)
		{
			return 1;
		}

		// Set header
		buffer[0] = 0x01;

		// Set key
		uint8_t* key = buffer + 1;
		memcpy(key, node->key, node->key[0] + 1);

		// Cleanup and copy pointers
		free(node->buffer - LWS_PRE);
		node->buffer = buffer;
		node->key = key;
		node->data = key + key_length + 2;
		node->allocated_length = buffer_length;
	}

	// Copy data
	*((uint16_t*)(node->key + key_length)) = htons(data_length);
	memcpy(node->data, data, data_length);
	node->current_length = buffer_length;

	// Success
	return 0;
}

/**
 * Queues a node to be written to a client
 * May be recursive if desired
 */
int node_queue(struct Node* node, struct session_data* psd, uint8_t recursive)
{
	int code = 0;
	// Queue response
	struct Action* action = addAction(psd->actions);
	// No error
	if(action)
	{
		// Node data fits in the pregenerated packet
		if(node->current_length > PREGEN_BUFFER_LENGTH)
		{
			action->type = PregeneratedRequest;
			memcpy(action->action.pregeneratedRequest.buffer + LWS_PRE,
				   node->buffer, node->current_length);
			action->action.pregeneratedRequest.length = node->current_length;

			// Manage children if recursive
			if(recursive)
			{
				for(int i = 0;i < MAX_CHILDREN;i++)
				{
					// No more children to check
					if(!(node->children[i]))
					{
						break;
					}
					// Queue child
					int code2 = node_queue(node->children[i], psd, recursive);
					// Handle no more actions
					if(code2)
					{
						code = 1;
					}
				}
			}
		}
		// Node data too long
		else
		{
			lwsl_notice("NOTICE: Node data too long.");
		}
	}
	// Too many actions
	else
	{
		lwsl_notice("NOTICE: Too many actions.");
		code = 1;
	}

	return code;
}

/**
 * Queues a node to have its ID and name written to a client
 * May be recursive if desired
 */
int node_queue_id(struct Node* node, struct session_data* psd, uint8_t recursive, uint8_t multilevel)
{
	int code = 0;
	// Queue and generate the response
	struct Action* action = addAction(psd->actions);
	// No error
	if(action)
	{
		action->type = PregeneratedRequest;
		uint8_t* buffer = action->action.pregeneratedRequest.buffer + LWS_PRE;
		int length = writeIDResponsePacket(buffer, BUFFER_LENGTH, node);
		action->action.pregeneratedRequest.length = length;

		// Manage children if recursive
		if(recursive)
		{
			for(int i = 0;i < MAX_CHILDREN;i++)
			{
				// No more children to check
				if(!(node->children[i]))
				{
					break;
				}
				// Queue child
				int code2 = node_queue_id(node->children[i], psd, recursive && multilevel, multilevel);
				// Handle no more actions
				if(code2 > 0)
				{
					code = 1;
				}
			}
		}
	}
	// Too many actions
	else
	{
		lwsl_notice("NOTICE: Too many actions.");
		code = 1;
	}

	return code;
}

/**
 * Adds a subscriber to the given node
 */
int node_add_subscriber(struct Node* node, struct session_data* psd, uint8_t recursive)
{
	// See if the subscription already exists
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(psd == node->subscribers[i])
		{
			node->recursive[i] = recursive;
			return i;
		}
	}

	// Add the subscription into the first avaliable spot
	int index = -1;
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(!(node->subscribers[i]))
		{
			node->subscribers[i] = psd;
			node->recursive[i] = recursive;
			index = i;
			break;
		}
	}

	// Bail if no space left
	if(index < 0)
	{
		return -1;
	}

	// Create event data to node link
	for(int i = 0;i < MAX_EVENT_SUBSCRIPTIONS;i++)
	{
		if(!(psd->subscribed[i]))
		{
			psd->subscribed[i] = node;
			return index;
		}
	}

	// No space left
	return -2;
}

/**
 * Removes a subscriber from the given node
 */
int node_remove_subscriber(struct Node* node, struct session_data* psd)
{
	// Search all subscribers
	int index = -1;
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(psd == node->subscribers[i])
		{
			node->subscribers[i] = 0;
			index = i;
			break;
		}
	}

	// Bail if no subscriber removed
	if(index < 0)
	{
		return -1;
	}

	// Remove event data to node link
	for(int i = 0;i < MAX_EVENT_SUBSCRIPTIONS;i++)
	{
		if(node == psd->subscribed[i])
		{
			psd->subscribed[i] = 0;
			return index;
		}
	}

	// No subscriber found
	return -2;
}

/**
 * Notify all subscribers that this node has updated
 */
int node_notify_subscribers(struct Node* node)
{
	int code = 0;

	// Search all subscribers
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(node->subscribers[i])
		{
			// Queue response
			int code2 = node_queue(node, node->subscribers[i], node->recursive[i]);
			// No error
			if(!code2)
			{
				// Request callback to write to client
				lws_callback_on_writable(node->subscribers[i]->wsi);
			}
			// Error
			{
				code = 1;
			}
		}
	}

	return code;
}

/**
 * Free all children of this node and then itself (recursive)
 */
void node_free_all(struct Node* node)
{
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		if(node->children[i])
		{
			node_free_all(node->children[i]);
		}
	}
	free(node);
}
