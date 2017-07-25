#include "subscription.h"

/**
 * Returns a pointer to a subscription matching the given node
 */
struct Subscription* subscription_get_with_node(struct Subscription* subscriptions, struct Node* node)
{
	// Search for the given node
	for(int i = 0;i < MAX_SUBSCRIPTIONS;i++)
	{
		if(subscriptions[i].node == node)
		{
			return &(subscriptions[i]);
		}
	}

	// No node found
	return 0;
}

/**
 * Returns a pointer to a subscription to be populated
 */
struct Subscription* subscription_get_empty(struct Subscription* subscriptions)
{
	// Search for empty subscription
	for(int i = 0;i < MAX_SUBSCRIPTIONS;i++)
	{
		if(!subscriptions[i].node)
		{
			return &(subscriptions[i]);
		}
	}

	// No empty subscriptions found
	return 0;
}
