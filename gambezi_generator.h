#ifndef GAMBEZI_GENERATOR_H
#define GAMBEZI_GENERATOR_H

#include <stdint.h>

void readIDRequestPacket(uint8_t* data, uint8_t** parent_key, uint8_t** name);
int writeIDResponsePacket(uint8_t* buffer, int limit, struct Node* node);

void readRefreshRateSetPacket(uint8_t* data, uint16_t* refresh_rate);
void readValueSetPacket(uint8_t* data, uint8_t** key, uint16_t* length, uint8_t** value);
void readValueRequestPacket(uint8_t* data, uint8_t** key, uint8_t* get_children);
void readSubscriptionUpdatePacket(uint8_t* data, uint8_t** key, uint8_t* get_children, uint16_t* refresh_skip);
int writeValueResponsePacket(uint8_t* buffer, int limit, struct Node* node);

#endif
