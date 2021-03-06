#pragma once

#include "tlwbe.h"

struct app {
	const gchar* eui;
	const gchar* name;
	const guint32 serial;
};

struct dev {
	const gchar* eui;
	const gchar* appeui;
	const gchar* key;
	const gchar* name;
	const guint32 serial;
};

struct session {
	const gchar* deveui;
	const gchar* devnonce;
	const gchar* appnonce;
	const gchar* devaddr;

};

struct keyparts {
	const gchar* key;
	const gchar* appnonce;
	const gchar* devnonce;
	// not really needed for the keys but useful
	const gchar* appeui;
	const gchar* deveui;
};

struct downlink {
	guint64 timestamp;
	guint32 deadline;
	const gchar* appeui;
	const gchar* deveui;
	guint8 port;
	const guint8* payload;
	gsize payloadlen;
	const gchar* token;
};

void database_init(struct context* cntx, const gchar* databasepath);

// app
void database_app_add(struct context* cntx, const struct app* app);
void database_app_update(struct context* cntx, const struct app* app);
void database_app_get(struct context* cntx, const char* eui,
		void (*callback)(const struct app* app, void* data), void* data);
void database_app_del(struct context* cntx, const char* eui);
void database_apps_list(struct context* cntx,
		void (*callback)(const char* eui, void* data), void* data);
// dev
void database_dev_add(struct context* cntx, const struct dev* dev);
void database_dev_update(struct context* cntx, const struct dev* dev);
void database_dev_get(struct context* cntx, const char* eui,
		void (*callback)(const struct dev* dev, void* data), void* data);
void database_dev_del(struct context* cntx, const char* eui);
void database_devs_list(struct context* cntx,
		void (*callback)(const char* eui, void* data), void* data);
//session
void database_session_add(struct context* cntx, const struct session* dev);
void database_session_get_deveui(struct context* cntx, const char* deveui,
		void (*callback)(const struct session* session, void* data), void* data);
void database_session_get_devaddr(struct context* cntx, const char* devaddr,
		void (*callback)(const struct session* session, void* data), void* data);
void database_session_del(struct context* cntx, const char* deveui);
int database_framecounter_down_getandinc(struct context* cntx,
		const char* devaddr);
void database_framecounter_up_set(struct context* cntx, const char* devaddr,
		int framecounter);
// dev + session
void database_keyparts_get(struct context*, const char* devaddr,
		void (*callback)(const struct keyparts* keyparts, void* data),
		void* data);
//downlinks
void database_downlink_add(struct context* cntx, struct downlink* downlink);
void database_downlinks_clean(struct context* cntx, guint64 timestamp);
