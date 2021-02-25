#include "command.h"
#include "mongoose.h"

#define LENGTH(X) (sizeof X / sizeof X[0])

void http_redirect_handler(struct mg_connection *con, const struct mg_http_message *msg, union arg *arg)
{
	const char *header_fmt = "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n";
	size_t len = strlen(arg->path) + strlen(header_fmt) - 1;
	char *header = malloc(len);
	snprintf(header, len, header_fmt, arg->path);
	mg_send(con, header, len);
	free(header);
}

void http_spawn_handler(struct mg_connection *con, const struct mg_http_message *msg, union arg *arg)
{
	if (spawn(arg->cmd.command)) {
		mg_http_reply(con, 500, "", "Error occured\r\n");
	}
}

#define BUFFER 256
void http_pipe_handler(struct mg_connection *con, const struct mg_http_message *msg, union arg *arg)
{
	char input[BUFFER];

	arg->cmd.input.ptr = input;
	arg->cmd.input.len = 0;

	int *len = (int *)&(arg->cmd.input.len);

	const struct mg_str *data = mg_strcmp(msg->method, mg_str("GET")) == 0 ? &(msg->query) : &(msg->body);
	for (int i = 0; arg->cmd.vars[i] != NULL; i++) {
		int var_len = mg_http_get_var(data, arg->cmd.vars[i], &input[*len], BUFFER - *len - 1);
		if (var_len < 0) {
			LOG(LL_ERROR, ("\npipe handler input failed!"));
			mg_http_reply(con, 500, "", "Error occured\r\n");
			return;
		}

		*len += var_len;
		input[*len] = '\n';
		(*len)++;
	}

	char output[BUFFER];
	arg->cmd.output.ptr = output;
	arg->cmd.output.len = BUFFER;

	struct pipe p;
	int nread = pipe_open(&p, &arg->cmd);
	if (nread < 0) {
		LOG(LL_ERROR, ("\npipe handler output failed!"));
		mg_http_reply(con, 500, "", "Error occured\r\n");
		return;
	} else {
		mg_http_reply(con, 200, "", "%.*s", nread, output);
	}
}
