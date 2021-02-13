#include "mongoose.h"

static const char *root_dir = ".";
static const char *listen_on = "http://localhost:8000";

static void handle_request(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
	if (ev == MG_EV_HTTP_MSG) {
		struct mg_http_serve_opts opts = {root_dir};
		mg_http_serve_dir(c, ev_data, &opts);
	}
	(void) fn_data;
}

int main(int argc, char *argv[]) {
	struct mg_mgr mgr;
	struct mg_connection *connection;

	// Initialise stuff
	mg_log_set("2");
	mg_mgr_init(&mgr);
	if ((connection = mg_http_listen(&mgr, listen_on, handle_request, &mgr)) == NULL) {
		LOG(LL_ERROR, ("Cannot listen on %s. Use http://ADDR:PORT or :PORT", listen_on));
		exit(EXIT_FAILURE);
	}

	// Start infinite event loop
	LOG(LL_INFO, ("Starting Mongoose v%s, serving [%s]", MG_VERSION, root_dir));
	for (;;) mg_mgr_poll(&mgr, 1000);
	mg_mgr_free(&mgr);

	return 0;
}
