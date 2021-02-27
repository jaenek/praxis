static const char *listen_on = "http://0.0.0.0:8000";
static const char *root_dir = "./examples/auth/public";

static struct http_handler http_handlers[] = {
	{ "GET", "/", NULL, http_redirect_handler, { .path = "index.html" } },
	{ "POST", "/auth", token_auth, http_pipe_handler, { .cmd = { .command = "echo 'Authorized successfuly!'", .vars = (const char *[]){ NULL } } } }
};
