// HTTP server config
static const char *listen_on = "http://0.0.0.0:8000";
static const char *root_dir = "./public";

static struct http_handler http_handlers[] = {
	{ "GET", "/", NULL, http_redirect_handler, { .path = "index.html" } }
};

// MQTT client config
// this define enables mqtt handling support
// #define ENABLE_MQTT
// url formatted address of mqtt broker mqtt://<user>:<password>@<host>:<port>
//static const char *mqtt_url = "mqtt://0.0.0.0:1883";
//
//static struct mqtt_handler mqtt_handlers[] = {
//    { "test/data", mqtt_pipe_handler, { .cmd = { .command  = "tee -a public/data" } } }
//};
