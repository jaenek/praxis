#define SRV_PATH "/home/jaenek/workspace/c/praxis/examples/blog/public/"

static handler handlers[] = {
	{ "POST",  "/", spawn_handler, "cd examples/blog && make" },
	{ "GET",  "/", redirect_handler, "/index.html" }
};
