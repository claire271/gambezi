#ifndef GAMBEZI_GENERATOR_H
#define GAMBEZI_GENERATOR_H

#include <stdint.h>

// Incoming packet IDs
#define PACKET_REQUEST_KEY 0
#define PACKET_SET_KEY 1
#define PACKET_SET_REFRESH_RATE 2
#define PACKET_UPDATE_KEY_SUBSCRIPTION 3
#define PACKET_REQUEST_KEY_VALUE 4

// Outgoing packet IDs
#define PACKET_RETURN_KEY_ID 0
#define PACKET_RETURN_KEY_VALUE 1
#define PACKET_RETURN_ERROR 2

void readIDRequestPacket(uint8_t* data, uint8_t** parent_key, uint8_t** name, uint8_t* get_children, uint8_t* get_children_all);
int writeIDResponsePacket(uint8_t* buffer, int limit, struct Node* node);

void readRefreshRateSetPacket(uint8_t* data, uint16_t* refresh_rate);
void readValueSetPacket(uint8_t* data, uint8_t** key, uint16_t* length, uint8_t** value);
void readValueRequestPacket(uint8_t* data, uint8_t** key, uint8_t* get_children);
void readSubscriptionUpdatePacket(uint8_t* data, uint8_t** key, uint8_t* get_children, uint16_t* refresh_skip);

int writeErrorPacket(uint8_t* buffer, int limit, const char* message);

#endif
