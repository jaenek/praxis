static const char* listen_on = "http://0.0.0.0:8000";
static const char* root_dir = ".";

//const char* input_vars[] = { "temp", "hum", NULL };

static struct handler handlers[] = {
	{ "POST",   "/sht", NULL, data_handler, { .cmd = { .command = "./append_date.sh | tee sht_data", .vars = (const char*[]){ "temp", "hum", NULL } } } },
	{ "POST",   "/bme", NULL, data_handler, { .cmd = { .command = "./append_date.sh | tee bme_data", .vars = (const char*[]){ "temp", "hum", "pres", NULL } } } }
};
