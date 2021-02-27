static const char *listen_on = "http://0.0.0.0:8000";
static const char *root_dir = "./public";

static struct http_handler http_handlers[] = {
	{ "GET", "/", NULL, http_redirect_handler, { .path = "mqtt_data" } }
};

#define ENABLE_MQTT 1

static const char *mqtt_url = "mqtt://0.0.0.0:1883";

static struct mqtt_handler mqtt_handlers[] = {
    { "test/data", mqtt_pipe_handler, { .cmd = { .command  = "tee -a public/mqtt_data" } } }
};
