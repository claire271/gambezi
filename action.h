#ifndef ACTION_H
#define ACTION_H

#include <libwebsockets.h>
#include <stdint.h>
#include "limits.h"

struct PregeneratedRequest
{
	uint8_t buffer[LWS_PRE + PREGEN_BUFFER_LENGTH];
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

/**
 * Initializes an ActionQueue that has already been alocated
 */
void initActionQueue(struct ActionQueue* actions);

/**
 * Adds an action to the action queue.
 * The pointer returned points to the action to be populated.
 */
struct Action* addAction(struct ActionQueue* actions);

/**
 * Gets the first queued action and takes it off of the queue
 * The pointer returned points to the action to be used
 */
struct Action* useAction(struct ActionQueue* actions);

#endif
