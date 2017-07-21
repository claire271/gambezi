#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include "limits.h"

struct Node
{
	uint8_t name[MAX_NAME_LENGTH];
	struct Node* children[MAX_CHILDREN];
	struct per_session_data_gambezi* subscribers[MAX_CLIENTS];
	
	uint8_t* buffer;
	uint16_t current_length;
	uint16_t allocated_length;

	uint8_t* key;
	uint8_t* data;
};

struct Node* node_init(const uint8_t* name, const uint8_t* parent_key, uint8_t id);
struct Node* node_traverse(struct Node* root_node, const uint8_t* key);
struct Node* get_node_with_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name);
void node_set_value(struct Node* node, const uint8_t* data, uint16_t data_length);

int node_add_subscriber(struct Node* node, struct per_session_data_gambezi* pss);
void node_notify_subscribers(struct Node* node);

#endif
