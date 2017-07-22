#include "subscription.h"

struct Subscription* subscription_get_with_node(struct Subscription* subscriptions, struct Node* node)
{
	for(int i = 0;i < MAX_SUBSCRIPTIONS;i++)
	{
		if(subscriptions[i].node == node)
		{
			return &(subscriptions[i]);
		}
	}
	return 0;
}

struct Subscription* subscription_get_empty(struct Subscription* subscriptions)
{
	for(int i = 0;i < MAX_SUBSCRIPTIONS;i++)
	{
		if(!subscriptions[i].node)
		{
			return &(subscriptions[i]);
		}
	}
	return 0;
}
