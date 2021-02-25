static const char *listen_on = "http://0.0.0.0:8000";
static const char *root_dir = "./public";

static struct http_handler http_handlers[] = {
	{ "GET", "/", NULL, http_redirect_handler, { .path = "temp" } }
};

// url formatted address of mqtt broker mqtt://<user>:<password@<host>:<port>
static const char *mqtt_url = "";

static struct mqtt_handler mqtt_handlers[] = {
    { "test/temp", mqtt_pipe_handler, { .cmd = { .command  = "./examples/data/append_date.sh | tee -a public/temp" } } }
};
