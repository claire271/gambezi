#ifndef ACTION_H
#define ACTION_H

#include <stdint.h>
#include "node.h"
#include "protocol_gambezi.h"

#define MAX_ACTIONS 8

struct KeyRequest
{
	uint8_t name[MAX_NAME_LENGTH];
	uint8_t key[MAX_KEY_LENGTH];
};

struct PregeneratedRequest
{
	uint8_t buffer[LWS_PRE + BUFFER_LENGTH];
	int length;
};

enum ActionType
{
	PregeneratedRequest, KeyRequest
};

union ActionContainer
{
	struct KeyRequest keyRequest;
	struct PregeneratedRequest pregeneratedRequest;
};

struct Action
{
	enum ActionType type;
	union ActionContainer action;
};

struct ActionQueue
{
	struct Action actions[MAX_ACTIONS];
	int tail_index;
	int length;
};

void initActionQueue(struct ActionQueue* actions);
struct Action* addAction(struct ActionQueue* actions);
struct Action* useAction(struct ActionQueue* actions);

#endif
