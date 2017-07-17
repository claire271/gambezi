#ifndef NODE_H
#define NODE_H

#include <stdint.h>

#define MAX_NAME_LENGTH 256
#define MAX_CHILDREN 256
#define MAX_DEPTH 256
#define MAX_KEY_LENGTH MAX_DEPTH

struct Node
{
	uint8_t name[MAX_NAME_LENGTH];
	struct Node* children[MAX_CHILDREN];
	uint16_t length;
	uint8_t* data;
};

struct Node* node_init(const uint8_t* name);
uint8_t node_get_id(struct Node* root_node, const uint8_t* parent_key, const uint8_t* name);

#endif
