#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include "limits.h"

struct Node
{
	// Node identification, children, and subscribers
	uint8_t name[MAX_NAME_LENGTH];
	struct Node* children[MAX_CHILDREN];
	struct session_data* subscribers[MAX_CLIENTS];
	uint8_t recursive[MAX_CLIENTS];
	
	// Buffer to be allocated
	uint8_t* buffer;
	uint16_t current_length;
	uint16_t allocated_length;

	// Pointers to locations within the buffer
	uint8_t* key;
	uint8_t* data;
};

/**
 * Allocates a node with a given name and type, and returns a pointer to it
 */
struct Node* node_init(const uint8_t* name, const uint8_t* parent_key, uint8_t id);

/**
 * Gets the node for a given key
 */
struct Node* node_traverse(struct Node* root_node, const uint8_t* key);

/**
 * Gets the ID of a node, creates it if it does not exist yet
 */
struct Node* get_node_with_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name);

/**
 * Sets the data of a given node
 */
int node_set_value(struct Node* node, const uint8_t* data, uint16_t data_length);

/**
 * Queues a node to be written to a client
 */
int node_queue(struct Node* node, struct session_data* psd, uint8_t recursive);

/**
 * Queues a node to have its ID and name written to a client
 * May be recursive if desired
 */
int node_queue_id(struct Node* node, struct session_data* psd, uint8_t recursive);

/**
 * Adds a subscriber to the given node
 */
int node_add_subscriber(struct Node* node, struct session_data* psd, uint8_t recursive);

/**
 * Removes a subscriber from the given node
 */
int node_remove_subscriber(struct Node* node, struct session_data* psd);

/**
 * Notify all subscribers that this node has updated
 */
void node_notify_subscribers(struct Node* node);

/**
 * Free all children of this node and then itself (recursive)
 */
void node_free_all(struct Node* node);

#endif
