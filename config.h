static const char* listen_on = "http://localhost:8000";
static const char* root_dir = ".";

//const char* input_vars[] = { "temp", "hum", NULL };

static struct handler handlers[] = {
	{ "GET",   "/",     NULL,       data_handler,     { .cmd = { .command = "tee -a incoming", .vars = (const char*[]){ "temp", "hum", NULL } } } },
	{ "GET",   "/auth", token_auth, data_handler,     { .cmd = { .command = "echo Authorized" } } }
};
