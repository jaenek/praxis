#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

#include "mongoose.h"
#include "sha256.h"

#define LENGTH(X)   (sizeof X / sizeof X[0])
#define STRFMT(str) (int) str.len, str.ptr

#include "tokens.h"

bool token_auth(struct mg_http_message* msg) {
	char token[32];

    mg_http_get_var(&msg->query, "access_token", token, sizeof(token));
	if (token[0] == '\0') {
		// no password provided
		return false;
	}

	SHA256_CTX ctx;
	BYTE buf[SHA256_BLOCK_SIZE];

	sha256_init(&ctx);
	sha256_update(&ctx, (BYTE*)token, strlen(token));
	sha256_final(&ctx, buf);

	for (int i = 0; i < LENGTH(tokens) ; i++) {
		if (memcmp(tokens[i], buf, SHA256_BLOCK_SIZE) == 0)
			return true;
	}

	return false;
}

union args {
	const char* path;
	const char* command;
};

struct handler{
	const char* method;
	const char* path;
	bool (*auth_func)(struct mg_http_message*);
	void (*handle_func)(struct mg_connection*, struct mg_http_message*, const union args*);
	const union args args;
};

void redirect_handler(struct mg_connection* con, struct mg_http_message* msg, const union args* args) {
	const char* header_fmt = "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n";
	size_t len = strlen(args->path) + strlen(header_fmt) - 1;
	char* header = malloc(len);
	snprintf(header, len, header_fmt, args->path);
	mg_send(con, header, len);
	free(header);
}

void spawn_handler(struct mg_connection* con, struct mg_http_message* msg, const union args* args) {
	pid_t pid;
	if ((pid = fork()) == 0) {
		setsid();
		execl("/usr/bin/sh", "sh", "-c", args->command, NULL);
		printf("praxis: execl %s", args->command);
		perror(" failed");
		exit(EXIT_SUCCESS);
	} else if(pid == -1) {
		mg_http_reply(con, 500, "", "Error occured\r\n");
		perror("praxis: fork() failed");
	}
}

#define CHUNK 1024
void data_handler(struct mg_connection* con, struct mg_http_message* msg, const union args* args) {
	struct { char* ptr; size_t len; size_t size; } output = {
		.ptr = malloc(CHUNK),
		.size = CHUNK
	};

	FILE *p = popen(args->command, "r");
	if (p == NULL) {
		mg_http_reply(con, 500, "", "Error occured\r\n");
		printf("praxis: popen %s", args->command);
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
#include "config.h"

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

static void handle_request(struct mg_connection* con, int event, void* data, void* fn_data) {
	(void) fn_data; // function specific data (not used)

	if (event != MG_EV_HTTP_MSG)
		return;

	struct mg_http_message* msg = (struct mg_http_message*) data;

	for (int i = 0; i < LENGTH(handlers); i++) {
		if (!mg_http_match_uri(msg, handlers[i].path)) {
			if (i == LENGTH(handlers) - 1) {
				mg_http_serve_dir(con, msg, &(struct mg_http_serve_opts){ root_dir, "#.shtml" });
				break;
			} else {
				continue;
			}
		}

		if (mg_strcmp(msg->method, mg_str(handlers[i].method)) == 0) {
			if (handlers[i].auth_func == NULL || handlers[i].auth_func(msg)) {
				LOG(LL_INFO, ("\n%.*s", STRFMT(msg->message)));
				handlers[i].handle_func(con, msg, &handlers[i].args);
			} else {
				char buf[40];
				LOG(LL_INFO, ("\nAuthorization denied: %s", mg_straddr(con, buf, sizeof(buf))));
				mg_printf(con, "HTTP/1.1 403 Denied\r\nContent-Length: 0\r\n\r\n");
			}
		}
	}
}

int main(void) {
	struct mg_mgr mgr;

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
