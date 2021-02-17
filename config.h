static const char* listen_on = "http://localhost:8000";
static const char* root_dir = ".";

static struct handler handlers[] = {
	{ "GET",   "/test", NULL,       redirect_handler, { .path = "/index.html" } },
	{ "GET",   "/",     NULL,       data_handler,     { .command = "ls -lh" } },
	{ "GET",   "/auth", token_auth, data_handler,     { .command = "echo Authorized" } }
};
