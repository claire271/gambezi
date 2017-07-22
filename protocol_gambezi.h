#ifndef PROTOCOL_GAMBEZI_H
#define PROTOCOL_GAMBEZI_H

#include "subscription.h"

struct per_vhost_data_gambezi
{
	struct Node* root_node;
	struct lws_context* context;
};

struct per_session_data_gambezi
{
	struct ActionQueue* actions;
	struct lws* wsi;
	uv_timer_t timer;
	struct Subscription subscriptions[MAX_SUBSCRIPTIONS];
};

#endif
