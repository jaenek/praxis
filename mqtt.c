#include "command.h"
#include "mongoose.h"

void mqtt_spawn_handler(struct mg_connection *con, const struct mg_mqtt_message *msg, union arg *arg) {
	if (spawn(arg->cmd.command)) {
		LOG(LL_ERROR, ("\nspawn handler failed!"));
		mg_http_reply(con, 500, "", "Error occured\r\n");
	}
}

#define BUFFER 256
void mqtt_pipe_handler(struct mg_connection *con, const struct mg_mqtt_message *msg, union arg *arg) {
	arg->cmd.input.ptr = (char *)msg->data.ptr;
	arg->cmd.input.len = msg->data.len;

	char output[BUFFER];
	arg->cmd.output.ptr = output;
	arg->cmd.output.len = BUFFER;

	struct pipe p;
	int nread = pipe_open(&p, &arg->cmd);
	if (nread < 0) {
		LOG(LL_ERROR, ("\npipe handler output failed!"));
	}
}
