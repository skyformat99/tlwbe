#include <json-glib/json-glib.h>
#include <mosquitto.h>
#include <string.h>

#include "control.h"
#include "database.h"

enum entity {
	ENTITY_APP, ENTITY_DEV, ENTITY_INVALID
};

enum action {
	ACTION_ADD,
	ACTION_UPDATE,
	ACTION_DEL,
	ACTION_GET,
	ACTION_LIST,
	ACTION_INVALID
};

static const char* entities[] = { CONTROL_ENTITY_APP, CONTROL_ENTITY_DEV };

static const char* actions[] = { CONTROL_ACTION_ADD, CONTROL_ACTION_UPDATE,
CONTROL_ACTION_DEL, CONTROL_ACTION_GET, CONTROL_ACTION_LIST };

static int control_app_add(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	if (!json_object_has_member(rootobj, CONTROL_JSON_NAME)
			|| !json_object_has_member(rootobj, CONTROL_JSON_EUI))
		return -1;

	const gchar* name = json_object_get_string_member(rootobj,
	CONTROL_JSON_NAME);
	const gchar* eui = json_object_get_string_member(rootobj, CONTROL_JSON_EUI);

	struct app a = { .name = name, .eui = eui };

	database_app_add(cntx, &a);

	return 0;
}

static int control_app_get(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_app_get(cntx, NULL);
	return 0;
}

static int control_app_update(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_app_update(cntx, NULL);
	return 0;
}

static int control_app_del(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_app_del(cntx, NULL);
	return 0;
}

static void control_apps_list_euicallback(const char* eui, void* data) {
	JsonBuilder* jsonbuilder = data;
	json_builder_add_string_value(jsonbuilder, eui);
}

static int control_apps_list(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	json_builder_set_member_name(jsonbuilder, "result");
	json_builder_begin_array(jsonbuilder);
	database_apps_list(cntx, control_apps_list_euicallback, jsonbuilder);
	json_builder_end_array(jsonbuilder);
	return 0;
}

static int control_dev_add(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_dev_add(cntx, NULL);
	return 0;
}

static int control_dev_update(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_dev_update(cntx, NULL);
	return 0;
}

static int control_dev_get(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_dev_get(cntx, NULL);
	return 0;
}

static int control_dev_del(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_dev_del(cntx, NULL);
	return 0;
}

static int control_devs_list(struct context* cntx, JsonObject* rootobj,
		JsonBuilder* jsonbuilder) {
	database_devs_list(cntx);
	return 0;
}

void control_onmsg(struct context* cntx, const struct mosquitto_message* msg,
		char** splittopic, int numtopicparts) {

	gchar* payload = NULL;
	char* entity = splittopic[2];
	char* action = splittopic[3];

	enum entity e = ENTITY_INVALID;
	for (int i = 0; i < G_N_ELEMENTS(entities); i++) {
		if (strcmp(entity, entities[i]) == 0)
			e = i;
	}
	if (e == ENTITY_INVALID) {
		g_message("invalid entity %s", entity);
		goto out;
	}

	enum action a = ACTION_INVALID;
	for (int i = 0; i < G_N_ELEMENTS(actions); i++) {
		if (strcmp(action, actions[i]) == 0)
			a = i;
	}
	if (a == ACTION_INVALID) {
		g_message("invalid action %s", action);
		goto out;
	}

	JsonParser* jsonparser = json_parser_new_immutable();
	if (!json_parser_load_from_data(jsonparser, msg->payload, msg->payloadlen,
	NULL)) {
		g_message("failed to parse control message json");
		goto out;
	}

	const gchar* token = NULL;

	JsonNode* rootnode = json_parser_get_root(jsonparser);
	if (json_node_get_node_type(rootnode) != JSON_NODE_OBJECT) {
		g_message("control message root should be an object");
		goto out;
	}
	JsonObject* rootobj = json_node_get_object(rootnode);
	if (!json_object_has_member(rootobj, CONTROL_JSON_TOKEN)) {
		g_message("control message does not contain token");
		goto out;
	}
	token = json_object_get_string_member(rootobj,
	CONTROL_JSON_TOKEN);

	JsonBuilder* jsonbuilder = json_builder_new();
	json_builder_begin_object(jsonbuilder);
	json_builder_set_member_name(jsonbuilder, CONTROL_JSON_TOKEN);
	json_builder_add_string_value(jsonbuilder, token);

	int code = -1;
	switch (e) {
	case ENTITY_APP:
		switch (a) {
		case ACTION_ADD:
			code = control_app_add(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_UPDATE:
			code = control_app_update(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_GET:
			code = control_app_get(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_DEL:
			code = control_app_del(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_LIST:
			code = control_apps_list(cntx, rootobj, jsonbuilder);
			break;
		}
		break;
	case ENTITY_DEV:
		switch (a) {
		case ACTION_ADD:
			code = control_dev_add(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_UPDATE:
			code = control_dev_update(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_GET:
			code = control_dev_get(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_DEL:
			code = control_dev_del(cntx, rootobj, jsonbuilder);
			break;
		case ACTION_LIST:
			code = control_devs_list(cntx, rootobj, jsonbuilder);
			break;
		}
		break;
	}

	json_builder_set_member_name(jsonbuilder, "code");
	json_builder_add_int_value(jsonbuilder, code);

	json_builder_end_object(jsonbuilder);

	JsonNode* responseroot = json_builder_get_root(jsonbuilder);
	JsonGenerator* generator = json_generator_new();
	json_generator_set_root(generator, responseroot);

	gsize payloadlen;
	payload = json_generator_to_data(generator, &payloadlen);

	json_node_free(responseroot);
	g_object_unref(generator);
	g_object_unref(jsonbuilder);

	mosquitto_publish(cntx->mosq, NULL,
	TLWBE_TOPICROOT"/"CONTROL_SUBTOPIC"/"CONTROL_RESULT, payloadlen, payload, 0,
	false);

	out: if (payload != NULL)
		g_free(payload);
}

void control_onbrokerconnect(struct context* cntx) {
	mosquitto_subscribe(cntx->mosq, NULL,
	TLWBE_TOPICROOT"/"CONTROL_SUBTOPIC"/"CONTROL_ENTITY_APP"/#", 0);
	mosquitto_subscribe(cntx->mosq, NULL,
	TLWBE_TOPICROOT"/"CONTROL_SUBTOPIC"/"CONTROL_ENTITY_DEV"/#", 0);
}

