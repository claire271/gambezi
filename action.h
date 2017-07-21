#ifndef ACTION_H
#define ACTION_H

#include <stdint.h>
#include "node.h"
#include "protocol_gambezi.h"

#define MAX_ACTIONS 8

struct PregeneratedRequest
{
	uint8_t buffer[LWS_PRE + BUFFER_LENGTH];
	int length;
};

struct DataReturnRequest
{
	struct Node* node;
};

enum ActionType
{
	PregeneratedRequest, DataReturnRequest
};

union ActionContainer
{
	struct PregeneratedRequest pregeneratedRequest;
	struct DataReturnRequest dataReturnRequest;
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
