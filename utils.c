#include <glib.h>

#include "utils.h"
#include "pktfwdbr.h"

gchar* utils_createtopic(const gchar* id, ...) {
	GString* topicstr = g_string_new(PKTFWDBR_TOPIC_ROOT"/");
	g_string_append(topicstr, id);

	va_list args;
	va_start(args, id);

	const gchar * part = va_arg(args, const gchar*);
	for (; part != NULL; part = va_arg(args, const gchar*)) {
		g_string_append(topicstr, "/");
		g_string_append(topicstr, part);
	}

	va_end(args);
	gchar* topic = g_string_free(topicstr, FALSE);
	return topic;
}