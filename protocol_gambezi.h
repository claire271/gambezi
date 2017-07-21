#ifndef PROTOCOL_GAMBEZI_H
#define PROTOCOL_GAMBEZI_H

struct per_vhost_data_gambezi
{
	struct Node* root_node;
};

struct per_session_data_gambezi
{
	struct ActionQueue* actions;
	struct lws* wsi;
};

#endif
