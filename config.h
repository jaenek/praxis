static const char* listen_on = "http://localhost:8000";
static const char* root_dir = ".";

static handler handlers[] = {
	{ "GET",   "/test", redirect_handler, { .s = "/index.html", } },
	{ "POST",  "/",     data_handler,     { .s = "ls -lh", } }
};
