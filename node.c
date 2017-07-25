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
struct Node* get_node_with_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name)
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
 * Adds a subscriber to the given node
 */
int node_add_subscriber(struct Node* node, struct session_data* psd)
{
	// See if the subscription already exists
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(psd == node->subscribers[i])
		{
			return i;
		}
	}

	// Add the subscription into the first avaliable spot
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(!(node->subscribers[i]))
		{
			node->subscribers[i] = psd;
			return i;
		}
	}

	// No space left
	return -1;
}

/**
 * Removes a subscriber from the given node
 */
int node_remove_subscriber(struct Node* node, struct session_data* psd)
{
	// Search all subscribers
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(psd == node->subscribers[i]) {
			node->subscribers[i] = 0;
			return i;
		}
	}

	// No subscriber removed
	return -1;
}

/**
 * Notify all subscribers that this node has updated
 */
void node_notify_subscribers(struct Node* node)
{
	// Search all subscribers
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		if(node->subscribers[i])
		{
			// Queue response
			struct session_data* psd = node->subscribers[i];
			struct Action* action = addAction(psd->actions);
			action->type = DataReturnRequest;
			action->action.dataReturnRequest.node = node;

			// Request callback to write to client
			lws_callback_on_writable(psd->wsi);
		}
	}
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
