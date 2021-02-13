#include <stdio.h>
#include <sys/wait.h>

#include "mongoose.h"

#define LENGTH(X)   (sizeof X / sizeof X[0])
#define STRFMT(str) (int) str.len, str.ptr

typedef struct {
	const char* method;
	const char* path;
	void (*func)(struct mg_connection* con, void* data, const void*);
	const void* args;
} handler;

void spawn_handler(struct mg_connection* con, struct mg_http_message* msg, const char* command) {
	pid_t pid;
	if ((pid = fork()) == 0) {
		setsid();
		execle("/usr/bin/sh", "sh", "-c", command, (char *)NULL);
		printf("praxis: execle %s", command);
		perror(" failed");
		exit(EXIT_SUCCESS);
	} else if(pid == -1) {
		mg_http_reply(con, 405, "", "Error occured\r\n");
		perror("praxis: fork() failed");
	}
	mg_http_reply(con, 200, "", "Ok!\r\n");
	waitpid(pid, NULL, WNOHANG);
}

#define CHUNK 256
void data_handler(struct mg_connection* con, struct mg_http_message* msg, const char* command) {
	struct { char* ptr; size_t len; size_t size; } output = {
		.ptr = malloc(CHUNK),
		.size = CHUNK
	};

	FILE *p = popen(command, "r");
	if (p == NULL) {
		printf("praxis: popen %s", command);
		perror(" failed");
	}

	char c;
	while ((c = fgetc(p)) != EOF) {
		if (output.len == output.size/sizeof(char) - 1)
			output.ptr = realloc(output.ptr, output.size +  CHUNK);

		output.ptr[output.len] = c;
		output.len++;
	}

	pclose(p);

	mg_http_reply(con, 200, "", "Output: %.*s\r\n", STRFMT(output));
	free(output.ptr);
}

void file_serve_handler(struct mg_connection* con, struct mg_http_message* msg, const char* dir_path) {
	struct mg_http_serve_opts opts = {dir_path};
	mg_http_serve_dir(con, msg, &opts);
}

// include user configuration after usable function/structure definitions
#include "config.h"

static void handle_request(struct mg_connection* con, int event, void* data, void* fn_data) {
	(void) fn_data; // function specific data (not used)

	struct mg_http_message* msg = (struct mg_http_message*) data;
	if (event == MG_EV_HTTP_MSG) {
		for (int i = 0; i < LENGTH(handlers); i++) {
			if (!mg_http_match_uri(msg, handlers[i].path)) {
				if (i == LENGTH(handlers) - 1) {
					mg_http_reply(con, 404, "", "Error: not found.\r\n");
					break;
				} else {
					continue;
				}
			}

			if (mg_strcmp(msg->method, mg_str(handlers[i].method)) == 0) {
				LOG(LL_INFO, ("\n%.*s", STRFMT(msg->message)));
				handlers[i].func(con, data, handlers[i].args);
				break;
			}
		}
	}
}

static const char *listen_on = "http://localhost:8000";

int main(void) {
	struct mg_mgr mgr;

	// Initialise stuff
	mg_log_set("2");
	mg_mgr_init(&mgr);
	if (mg_http_listen(&mgr, listen_on, handle_request, &mgr) == NULL) {
		LOG(LL_ERROR, ("Cannot listen on %s. Use http://ADDR:PORT or :PORT", listen_on));
		exit(EXIT_FAILURE);
	}

	// Start infinite event loop
	LOG(LL_INFO, ("Starting Mongoose v%s", MG_VERSION));
	for (;;) mg_mgr_poll(&mgr, 1000);
	mg_mgr_free(&mgr);

	return 0;
}
