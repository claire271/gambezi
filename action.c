#include "action.h"

void initActionQueue(struct ActionQueue* actions)
{
	actions->tail_index = 0;
	actions->length = 0;
}

struct Action* addAction(struct ActionQueue* actions)
{
	// Bail if the queue is full
	if(actions->length >= MAX_ACTIONS)
	{
		lwsl_err("ERROR: Action queue full");
		return 0;
	}

	// Get next available action
	int index = (actions->tail_index + actions->length) % MAX_ACTIONS;
	actions->length++;
	return &(actions->actions[index]);
}

struct Action* useAction(struct ActionQueue* actions)
{
	// Bail if the queue is empty
	if(actions->length <= 0)
	{
		return 0;
	}

	// Get earliest action added
	int index = actions->tail_index;
	actions->tail_index = (actions->tail_index + 1) % MAX_ACTIONS;
	actions->length--;
	return &(actions->actions[index]);
}
