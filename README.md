### 🐻‍❄️🏳️‍⚧️ Logrin
#### *Modern, async-aware logging framework for C++20*
**Logrin** is a newer, modern, async-aware logging framework for C++20. It was designed and built to provide rich, structured, contextual-aware diagnostics for C++ applications with feature parity of Go's [`logrus`] and Rust's [`tracing`] libraries.

[`logrus`]: https://github.com/sirupsen/logrus
[`tracing`]: https://docs.rs/tracing

## Features
- **Structured Logging**
    - key/value pairs attached to log records
    - thread-local and task-local context propagation
    - inclusion of span/trace IDs for observability
- **Multiple Backends / Sinks**
    - Console (human-readable, prettified)
    - JSON logs (for structured output)
    - OpenTelemetry support (tracing, logs)
    - File sinks with rotation and retention
    - Custom sinks via `logrin::Sink`
- **Asynchronous first**
    - Fully compatible with `kj::Promise`
    - Non-blocking log emission
    - Optional batching for high-performance output
- **Extensible**
  - Easily add new sinks or custom log processing
  - Minimal dependencies, header-only optional
  - No RTTI or exceptions required (exception-free mode supported)

## Examples
### Single-logger construction
```cpp
#include <logrin/Sinks/Console.h>
#include <logrin/Logger.h>

auto logger = logrin::Logger::Create("some logger here");
logger.AddSink<logrin::sinks::Console>(logrin::sinks::ConsoleOptions {
    .Timestamp = true,
    .SourceInformation = true
});

logger.Info("logrin was initialized");
logger.Debug("disk is almost full").With("disk", "/dev/sda1");
```

### Logging factory
```cpp
#include <logrin/Sinks/Console.h>
#include <logrin/LogFactory.h>

logrin::LogFactory::Initialize({
    new logrin::sinks::Console()
});

LOG_INFO("some logger here", "we are doing things").With("things", true);
```

## License
**logrin** is released under the **MIT License**, view the [`LICENSE`](./LICENSE) file for more information.
