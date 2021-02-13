static handler handlers[] = {
	{ "GET",   "/", file_serve_handler, "." },
	{ "POST",  "/", spawn_handler, "echo test123" }
};
