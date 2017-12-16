#pragma once

#include <glib.h>
#include <sqlite3.h>

#define TLWBE_TOPICROOT "tlwbe"

struct context {
	struct mosquitto* mosq;
	const gchar* mqtthost;
	gint mqttport;
	GIOChannel* mosqchan;
	guint mosqsource;
	sqlite3 *db;
};
