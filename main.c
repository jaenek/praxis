#include <stdio.h>
#include <sys/wait.h>

#include "mongoose.h"

#define LENGTH(X)   (sizeof X / sizeof X[0])
#define STRFMT(str) (int) str.len, str.ptr

typedef struct {
	const char* method;
	const char* path;
	void (*func)(struct mg_connection* con, struct mg_http_message* data, const char*);
	const void* args;
} handler;

void serve_dir_handler(struct mg_connection* con, struct mg_http_message* msg, const char* dir_path) {
	struct mg_http_serve_opts opts = {dir_path};
	mg_http_serve_dir(con, msg, &opts);
}

void redirect_handler(struct mg_connection* con, struct mg_http_message* msg, const char* url) {
	const char* header_fmt = "HTTP/1.1 302 Moved Permanently\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n";
	size_t len = strlen(url) + strlen(header_fmt)-1;
	char* header = malloc(len);
	snprintf(header, len, header_fmt, url);
	mg_send(con, header, len);
	free(header);
}

void spawn_handler(struct mg_connection* con, struct mg_http_message* msg, const char* command) {
	pid_t pid;
	if ((pid = fork()) == 0) {
		setsid();
		execl("/usr/bin/sh", "sh", "-c", command);
		printf("praxis: execl %s", command);
		perror(" failed");
		exit(EXIT_SUCCESS);
	} else if(pid == -1) {
		mg_http_reply(con, 500, "", "Error occured\r\n");
		perror("praxis: fork() failed");
	}
	mg_http_reply(con, 200, "", "Ok!\r\n");
	waitpid(pid, NULL, WNOHANG);
}

#define CHUNK 1024
void data_handler(struct mg_connection* con, struct mg_http_message* msg, const char* command) {
	struct { char* ptr; size_t len; size_t size; } output = {
		.ptr = malloc(CHUNK),
		.size = CHUNK
	};

	FILE *p = popen(command, "r");
	if (p == NULL) {
		mg_http_reply(con, 500, "", "Error occured\r\n");
		printf("praxis: popen %s", command);
		perror(" failed");
		free(output.ptr);
		return;
	}

	char c;
	while ((c = fgetc(p)) != EOF) {
		if (output.len == output.size/sizeof(char) - 1) {
			output.size += CHUNK;
			output.ptr = realloc(output.ptr, output.size);
		}

		output.ptr[output.len] = c;
		output.len++;
	}

	pclose(p);

	mg_http_reply(con, 200, "", "Output: %.*s\r\n", STRFMT(output));
	free(output.ptr);
}

// include user configuration after usable function/structure definitions
#include "examples/blog/config.h"
#ifndef SRV_PATH
#define SRV_PATH "."
#endif

static void handle_request(struct mg_connection* con, int event, void* data, void* fn_data) {
	(void) fn_data; // function specific data (not used)

	if (event != MG_EV_HTTP_MSG)
		return;

	struct mg_http_message* msg = (struct mg_http_message*) data;

	for (int i = 0; i < LENGTH(handlers); i++) {
		if (!mg_http_match_uri(msg, handlers[i].path)) {
			if (i == LENGTH(handlers) - 1) {
				serve_dir_handler(con, msg, SRV_PATH);
				break;
			} else {
				continue;
			}
		}

		if (mg_strcmp(msg->method, mg_str(handlers[i].method)) == 0) {
			LOG(LL_INFO, ("\n%.*s", STRFMT(msg->message)));
			handlers[i].func(con, data, handlers[i].args);
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
