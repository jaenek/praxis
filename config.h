static handler handlers[] = {
	{ "GET",   "/test", redirect_handler, "/" },
	{ "GET",   "/", serve_dir_handler, "." },
	{ "POST",  "/", data_handler, "ls -lh" }
};
