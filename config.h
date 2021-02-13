#define SH(cmd) { "sh", "-c", cmd, NULL }

static handler handlers[] = {
	{ "GET",   "/", hello_handler, NULL },
	{ "POST",  "/", spawn_handler, SH("xsetroot -name test") }
};
