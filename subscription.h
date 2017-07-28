#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <libwebsockets.h>
#include <stdint.h>
#include "limits.h"

struct Subscription
{
	struct Node* node;
	uint8_t recursive;
	uint16_t period;
	uint16_t count;
};

/**
 * Returns a pointer to a subscription matching the given node
 */
struct Subscription* subscription_get_with_node(struct Subscription* subscriptions, struct Node* node);

/**
 * Returns a pointer to a subscription to be populated
 */
struct Subscription* subscription_get_empty(struct Subscription* subscriptions);

#endif
