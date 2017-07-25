#ifndef PROTOCOL_GAMBEZI_H
#define PROTOCOL_GAMBEZI_H

#include "subscription.h"

struct vhost_data
{
	struct Node* root_node;
	struct lws_context* context;
};

struct session_data
{
	struct ActionQueue* actions;
	struct lws* wsi;
	uv_timer_t timer;
	struct Subscription subscriptions[MAX_SUBSCRIPTIONS];
};

#endif
