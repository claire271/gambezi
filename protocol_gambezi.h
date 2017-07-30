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
	struct TimerHolder* timer_holder;
	struct Subscription subscriptions[MAX_SUBSCRIPTIONS];
};

struct TimerHolder
{
	// DO NOT REARRAGE
	// Code relies on timer being the first element of this struct
	uv_timer_t timer;
	struct session_data* psd;
};

#endif
