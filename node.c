#include <libwebsockets.h>
#include <stdlib.h>
#include "node.h"
#include <stdio.h>
#include <string.h>
#include "arpa/inet.h"

// Allocates a node with a given name and type, and returns a pointer to it
struct Node* node_init(const uint8_t* name, const uint8_t* parent_key, uint8_t id)
{
	// Construct node
	struct Node* node = malloc(sizeof(struct Node));
	if(!node)
	{
		lwsl_err("ERROR: Unable to allocate memory for a node");
		return 0;
	}

	// Assign name
	memcpy(node->name, name, name[0] + 1);

	// Zero children nodes
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		node->children[i] = 0;
	}

	// Allocate buffer
	int key_length = parent_key[0] + 2;
	int buffer_length = 1 + key_length + 2;
	node->buffer = malloc(LWS_PRE + buffer_length) + LWS_PRE;

	// Set header
	node->buffer[0] = 0x01;

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

	return node;
}

// Gets the node for a given key
struct Node* node_traverse(struct Node* root_node, const uint8_t* key)
{
	int depth = key[0];
	for(;depth > 0;depth--)
	{
		key++;
		root_node = root_node->children[*key];
		if(!root_node)
		{
			lwsl_err("ERROR: No node present at given key");
			return 0;
		}
	}

	return root_node;
}

// Gets the ID of a node, creates it if it does not exist yet
struct Node* get_node_with_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name)
{
	// Traverse tree to immediate parent
	struct Node* parent_node = node_traverse(root_node, parent_key);

	// Search for the node
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		// Node not found
		if(!(parent_node->children[i]))
		{
			parent_node->children[i] = node_init(name, parent_node->key, i);
			return parent_node->children[i];
		}
		// Node found
		if(!(memcmp(name, parent_node->children[i]->name, name[0] + 1)))
		{
			return parent_node->children[i];
		}
	}

	lwsl_err("ERROR: Unable to create an additional child node");
	return 0;
}

void node_set_value(struct Node* node, const uint8_t* data, uint16_t data_length)
{
	// Calculate lengths
	int key_length = node->key[0] + 1;
	int buffer_length = 1 + key_length + 2 + data_length;

	// Check if current allocation is large enough
	if(buffer_length > node->allocated_length)
	{
		// Allocate buffer
		uint8_t* buffer = malloc(LWS_PRE + buffer_length) + LWS_PRE;

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
}
