### 🐻‍❄️🏳️‍⚧️ Logrin
#### *Modern, async-aware logging framework for C++20*
**Logrin** is a newer, modern, async-aware logging framework for C++20. It was designed and built to provide rich, structured, contextual-aware diagnostics for C++ applications with feature parity of Go's [`logrus`] and Rust's [`tracing`] libraries.

[`logrus`]: https://github.com/sirupsen/logrus
[`tracing`]: https://docs.rs/tracing

## Features
- **Structured Logging**
    - key/value pairs attached to log records
    <!-- - thread-local and task-local context propagation
    - inclusion of span/trace IDs for observability -->
- **Multiple Backends / Sinks**
    - Console (human-readable, prettified)
    - JSON logs (for structured output)
    <!-- - OpenTelemetry support (tracing, logs)
    - File sinks with rotation and retention -->
    - Custom sinks via `logrin::Sink`
- **Asynchronous first**
    <!-- - Fully compatible with `kj::Promise` -->
    - Non-blocking log emission with `logrin::AsyncSink`
    - Optional batching for high-performance output
- **Extensible**
  - Easily add new sinks or custom log processing
  - Minimal dependencies
  - No RTTI or exceptions required (exception-free mode supported)

## Examples
### Single-logger construction
```cpp
#include <logrin/Sinks/Console/Formatter/Json.h>
#include <logrin/Sinks/Console.h>
#include <logrin/Logger.h>

using logrin::sinks::Console;
using logrin::sinks::console::formatters::Json;

logrin::Logger log("a logger");
log.AddSink(Console{}.WithFormatter<Json>());

logger.Info("logrin was initialized");
logger.Debug("disk is almost full", FIELD("disk", "/dev/sda1"));
```

### Logging factory
```cpp
#include <logrin/Sinks/Console/Formatter/Json.h>
#include <logrin/Sinks/Console.h>
#include <logrin/LogFactory.h>

using logrin::sinks::console::formatters::Json;

auto* console = new logrin::sinks::Console();
console->WithFormatter<Json>();

logrin::LogFactory::Init({
    // the factory will own this allocation and on shutdown, it'll flush
    // any output and deallocate the memory
    console
});

auto log = logrin::LogFactory::Get("a logger");
LOG_INFO(log, "logrin was initialized");
LOG_DEBUG(log, "disk is almost full", FIELD("disk", "/dev/sda1"));
```

## License
**logrin** is released under the **MIT License**, view the [`LICENSE`](./LICENSE) file for more information.
