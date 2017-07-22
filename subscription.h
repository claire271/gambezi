#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <libwebsockets.h>
#include <stdint.h>
#include "limits.h"

struct Subscription
{
	struct Node* node;
	uint16_t period;
	uint16_t count;
};

struct Subscription* subscription_get_with_node(struct Subscription* subscriptions, struct Node* node);
struct Subscription* subscription_get_empty(struct Subscription* subscriptions);

#endif
