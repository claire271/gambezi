#ifndef GAMBEZI_GENERATOR_H
#define GAMBEZI_GENERATOR_H

#include <stdint.h>
#include "node.h"

void readIDRequestPacket(uint8_t* data, uint8_t** parent_key, uint8_t** name);
int writeIDResponsePacket(uint8_t* buffer, int limit, struct Node* node);

void readValueSetPacket(uint8_t* data, uint8_t** key, uint16_t* length, uint8_t** value);
void readValueRequestPacket(uint8_t* data, uint8_t** key, uint8_t* get_children);
int writeValueResponsePacket(uint8_t* buffer, int limit, struct Node* node);

#endif
