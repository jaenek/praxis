# Praxis

- - -

According to wikipedia definition `praxis` is the process of exercising ideas or theories.
**Praxis** web server is the application of unix philosophy which idealises minimalist
software that:
  - does one thing and does it well,
  - works together,
  - handles text streams.

## How it works

- - -

Configuration of the web server can look like this:
```
#define SRV_PATH "examples/blog/public/"

static handler handlers[] = {
	{ "POST",  "/", spawn_handler, "cd examples/blog && make" },
	{ "GET",  "/", redirect_handler, "/index.html" }
}
```

As you can probably tell `SRV_PATH` describes the directory with statically served files.
User defines handlers array which tells the web server what to do on each request.
So far the following handlers have been introduced:
  - `serve_dir_handler` - default handler when the requested path isn't found in
    handlers array,
  - `redirect_handler` - redirects on requested path to other location (302 code).
  - `spawn_handler` - spawns a subshell and executes the provided command can be
	used to for example generate static files.
  - `data_handler` - just as the `spawn_handler` spawns a subshell and executes the
    provided command but sends its output to a client.
