#include <string.h>
#include <arpa/inet.h>
#include "node.h"
#include "gambezi_generator.h"

void readIDRequestPacket(uint8_t* data, uint8_t** parent_key, uint8_t** name, uint8_t* get_children, uint8_t* get_children_all)
{
	(*parent_key) = data + 2;
	(*name) = data + (*parent_key)[0] + 3;
	(*get_children) = !!(data[1] & 0x01);
	(*get_children_all) = !!(data[1] & 0x02);
}

int writeIDResponsePacket(uint8_t* buffer, int limit, struct Node* node)
{
	int length = 0;
	// Packet header
	buffer[length++] = PACKET_RETURN_KEY_ID;
	// Key
	memcpy(buffer + length, node->key, node->key[0] + 1);
	length += node->key[0] + 1;
	// Name
	memcpy(buffer + length, node->name, node->name[0] + 1);
	length += node->name[0] + 1;

	return length;
}

int writeErrorPacket(uint8_t* buffer, int limit, const char* message)
{
	int length = 0;
	// Packet header
	buffer[length++] = PACKET_RETURN_ERROR;
	// Message length
	int string_length = strlen(message);
	if(string_length > 255)
	{
		string_length = 255;
	}
	buffer[length++] = string_length;
	// Message
	memcpy(buffer + length, message, string_length);
	length += string_length;

	return length;
}

void readRefreshRateSetPacket(uint8_t* data, uint16_t* refresh_rate)
{
	(*refresh_rate) = ntohs(*((uint16_t*)(data + 1)));
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
