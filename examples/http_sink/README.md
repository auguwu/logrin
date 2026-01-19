The **http_sink** example shows the sink: `logrin::sinks::Http` built ontop of libcurl.

The sink doesn't initialize `libcurl` data structures, you are on your own... or you can use the [`Noelware.Violet.Networking`](https://github.com/Noelware/Violet.Networking) library and use the `CurlGlobal` RAII guard.

You can run any HTTP server and just run `bazel run //examples/http_sink http://localhost:{port}` or expose the server with port `10000`.
