#include <string.h>
#include <arpa/inet.h>
#include "node.h"
#include "gambezi_generator.h"

void readIDRequestPacket(uint8_t* data, uint8_t** parent_key, uint8_t** name)
{
	(*parent_key) = data + 1;
	(*name) = data + (*parent_key)[0] + 2;
}

int writeIDResponsePacket(uint8_t* buffer, int limit, struct Node* node)
{
	int length = 0;
	// Packet header
	buffer[length++] = 0x00;
	// Key
	memcpy(buffer + length, node->key, node->key[0] + 1);
	length += node->key[0] + 1;
	// Name
	memcpy(buffer + length, node->name, node->name[0] + 1);
	length += node->name[0] + 1;

	return length;
}

void readValueSetPacket(uint8_t* data, uint8_t** key, uint16_t* length, uint8_t** value)
{
	(*key) = data + 1;
	(*length) = ntohs(*((uint16_t*)(data + (*key)[0] + 2)));
	(*value) = data + (*key)[0] + 4;
}

void readValueRequestPacket(uint8_t* data, uint8_t** key, uint8_t* get_children)
{
	(*key) = data + 2;
	(*get_children) = !!(data[1] & (0x01 << 0));
}

void readSubscriptionUpdatePacket(uint8_t* data, uint8_t** key, uint8_t* set_children, uint16_t* refresh_skip)
{
	(*key) = data + 4;
	(*set_children) = !!(data[1] & (0x01 << 0));
	(*refresh_skip) = ntohs(*((uint16_t*)(data + 2)));
}

int writeValueResponsePacket(uint8_t* buffer, int limit, struct Node* node)
{
	int length = 0;
	// Packet header
	buffer[length++] = 0x01;
	// Key
	memcpy(buffer + length, node->key, node->key[0] + 1);
	length += node->key[0] + 1;
	// Length
	*((uint16_t*)(buffer + length)) = htons(node->current_length - 5);
	length += 2;
	// Data
	memcpy(buffer + length, node->data, node->current_length - 5);
	length += node->current_length - 5;

	return length;
}
