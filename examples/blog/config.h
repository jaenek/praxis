static const char* listen_on = "http://0.0.0.0:8000";
static const char* root_dir = "./examples/blog/public";

static struct http_handler http_handlers[] = {
	{ "GET",  "/",         NULL, http_redirect_handler, { .path = "/index.shtml" } },
	{ "GET",  "/make",     NULL, http_spawn_handler,    { .cmd = { .command = "cd examples/blog && make" } } },
	{ "GET",  "/make",     NULL, http_redirect_handler, { .path = "/index.shtml" } },
	{ "GET",  "/list_dir", NULL, http_pipe_handler,     { .cmd = {.command = "ls -lh" } } },
};
