#include "action.h"

/**
 * Initializes an ActionQueue that has already been alocated
 */
void initActionQueue(struct ActionQueue* actions)
{
	actions->tail_index = 0;
	actions->length = 0;
}

/**
 * Adds an action to the action queue.
 * The pointer returned points to the action to be populated.
 */
struct Action* addAction(struct ActionQueue* actions)
{
	// Bail if the queue is full
	if(actions->length >= MAX_ACTIONS)
	{
		return 0;
	}

	// Get next available action and increment length
	int index = (actions->tail_index + actions->length) % MAX_ACTIONS;
	actions->length++;
	return &(actions->actions[index]);
}

/**
 * Gets the first queued action and takes it off of the queue
 * The pointer returned points to the action to be used
 */
struct Action* useAction(struct ActionQueue* actions)
{
	// Bail if the queue is empty
	if(actions->length <= 0)
	{
		return 0;
	}

	// Get earliest action added, moves tail index, and decrement length
	int index = actions->tail_index;
	actions->tail_index = (actions->tail_index + 1) % MAX_ACTIONS;
	actions->length--;
	return &(actions->actions[index]);
}
