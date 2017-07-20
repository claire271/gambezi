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

	// Assign key
	memcpy(node->key, parent_key, parent_key[0] + 1);
	node->key[0]++;
	node->key[node->key[0]] = id;

	// Zero children nodes
	for(int i = 0;i < MAX_CHILDREN;i++)
	{
		node->children[i] = 0;
	}

	// Set the data
	node->allocated_length = 0;
	node->current_length = 0;
	node->data = 0;

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

void node_set_value(struct Node* node, const uint8_t* data, uint16_t length)
{
	// Check if current allocation is large enough
	if(length > node->allocated_length)
	{
		free(node->data);
		node->data = malloc(length);
		node->allocated_length = length;
	}

	// Copy data
	memcpy(node->data, data, length);
	node->current_length = length;
}
