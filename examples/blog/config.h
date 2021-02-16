static const char* listen_on = "http://localhost:8000";
static const char* root_dir = "examples/blog/public";

static struct handler handlers[] = {
	{ "GET",  "/",         redirect_handler, { .path = "/index.html" } },
	{ "GET",  "/make",     spawn_handler,    { .command = "cd examples/blog && make" } },
	{ "GET",  "/make",     redirect_handler, { .path = "/index.html" } },
	{ "GET",  "/list_dir", data_handler,     { .command = "ls -lh" } },
};
