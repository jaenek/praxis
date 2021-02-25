#include <sys/wait.h>

#include "command.h"
#include "mongoose.h"
#include "sha256.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define STRFMT(str) (int)str.len, str.ptr

struct http_handler {
	const char *method;
	const char *path;
	int (*auth_func)(struct mg_http_message *);
	void (*handle_func)(struct mg_connection *, const struct mg_http_message *, union arg *);
	union arg arg;
};

void http_redirect_handler(struct mg_connection *, const struct mg_http_message *, union arg *);
void http_spawn_handler(struct mg_connection *, const struct mg_http_message *, union arg *);
void http_pipe_handler(struct mg_connection *, const struct mg_http_message *, union arg *);

struct mqtt_handler {
	const char *topic;
	void (*handle_func)(struct mg_connection *, const struct mg_mqtt_message *, union arg *);
	union arg arg;
};

void mqtt_spawn_handler(struct mg_connection *, const struct mg_mqtt_message *, union arg *);
void mqtt_pipe_handler(struct mg_connection *, const struct mg_mqtt_message *, union arg *);

#include "tokens.h"

bool token_auth(struct mg_http_message *msg)
{
	char token[32];
	token[0] = '\0';

	mg_http_get_var(&msg->query, "access_token", token, sizeof(token));
	if (token[0] == '\0') {
		// no password provided
		return false;
	}

	SHA256_CTX ctx;
	BYTE buf[SHA256_BLOCK_SIZE];

	sha256_init(&ctx);
	sha256_update(&ctx, (BYTE *)token, strlen(token));
	sha256_final(&ctx, buf);

	for (int i = 0; i < LENGTH(tokens); i++) {
		if (memcmp(tokens[i], buf, SHA256_BLOCK_SIZE) == 0)
			return true;
	}

	return false;
}

// include user configuration after usable function/structure definitions
#include "config.h"

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	errno = saved_errno;
}

void handle_http(struct mg_connection *con, struct mg_http_message *msg)
{
	for (int i = 0; i < LENGTH(http_handlers); i++) {
		if (strcmp(http_handlers[i].method, "MQTT") == 0)
			continue;

		if (!mg_http_match_uri(msg, http_handlers[i].path)) {
			if (i == LENGTH(http_handlers) - 1) {
				mg_http_serve_dir(con, msg, &(struct mg_http_serve_opts){root_dir, "#.shtml"});
				break;
			} else {
				continue;
			}
		}

		if (mg_strcmp(msg->method, mg_str(http_handlers[i].method)) == 0) {
			if (http_handlers[i].auth_func == NULL || http_handlers[i].auth_func(msg)) {
				LOG(LL_INFO, ("\n%.*s", STRFMT(msg->message)));
				http_handlers[i].handle_func(con, (const struct mg_http_message *)msg,
							     &http_handlers[i].arg);
			} else {
				char buf[40];
				LOG(LL_INFO, ("\nAuthorization denied: %s", mg_straddr(con, buf, sizeof(buf))));
				mg_printf(con, "HTTP/1.1 403 Denied\r\nContent-Length: 0\r\n\r\n");
			}
		}
	}
}

void handle_mqtt_open(struct mg_connection *con)
{
	for (int i = 0; i < LENGTH(mqtt_handlers); i++) {
		struct mg_str topic = mg_str(mqtt_handlers[i].topic);
		mg_mqtt_sub(con, &topic);
		LOG(LL_INFO, ("SUBSCRIBED to %.*s", STRFMT(topic)));
	}
}

void handle_mqtt(struct mg_connection *con, struct mg_mqtt_message *msg)
{
	for (int i = 0; i < LENGTH(mqtt_handlers); i++) {
		if (mg_strcmp(msg->topic, mg_str(mqtt_handlers[i].topic)) == 0) {
			LOG(LL_INFO, ("RECEIVED %.*s <- %.*s", STRFMT(msg->data), STRFMT(msg->topic)));
			mqtt_handlers[i].handle_func(con, msg, &mqtt_handlers[i].arg);
		}
	}
}

static void handle_request(struct mg_connection *con, int event, void *data, void *fn_data)
{
	(void)fn_data; // function specific data (not used)

	switch (event) {
	case MG_EV_ERROR: {
		LOG(LL_ERROR, ("%p %s", con->fd, (char *)data));
	} break;

	case MG_EV_HTTP_MSG: {
		handle_http(con, (struct mg_http_message *)data);
	} break;

	case MG_EV_MQTT_OPEN: {
		handle_mqtt_open(con);
	} break;

	case MG_EV_MQTT_MSG: {
		handle_mqtt(con, (struct mg_mqtt_message *)data);
	} break;

	default:
		LOG(LL_DEBUG, ("Event not handled %p %s", con->fd, (char *)data));
		break;
	}
}

int main(void)
{
	// reap all dead processes
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// Initialise stuff
	struct mg_mgr mgr;
	struct mg_mqtt_opts opts = {
		.client_id = mg_str("praxis"),
	};

	mg_log_set("2");

	mg_mgr_init(&mgr);
	mg_mqtt_connect(&mgr, mqtt_url, &opts, handle_request, NULL);
	if (mg_http_listen(&mgr, listen_on, handle_request, &mgr) == NULL) {
		LOG(LL_ERROR, ("Cannot listen on %s. Use http://ADDR:PORT or :PORT", listen_on));
		exit(EXIT_FAILURE);
	}

	// Start infinite event loop
	LOG(LL_INFO, ("Starting Mongoose v%s", MG_VERSION));
	while (true)
		mg_mgr_poll(&mgr, 1000);
	mg_mgr_free(&mgr);

	return 0;
}
