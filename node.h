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

#endif
