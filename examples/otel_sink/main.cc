// 🐻‍❄️🏳️‍⚧️ Logrin: Modern, async-aware logging framework for C++20
// Copyright (c) 2026 Noel Towa <cutie@floofy.dev>, et al.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <logrin/LogFactory.h>
#include <logrin/Sinks/OpenTelemetry.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/logs/batch_log_record_processor.h>
#include <opentelemetry/sdk/logs/logger_provider.h>
#include <violet/Violet.h>

namespace {

namespace otlp = opentelemetry::exporter::otlp;

void initOpenTelemetry(const char* url)
{
    otlp::OtlpHttpLogRecordExporterOptions opts;
    opts.url = url;

    auto exporter = std::make_unique<otlp::OtlpHttpLogRecordExporter>(opts);

    auto provider = std::make_shared<opentelemetry::sdk::logs::LoggerProvider>();
    provider->AddProcessor(std::make_unique<opentelemetry::sdk::logs::BatchLogRecordProcessor>(VIOLET_MOVE(exporter)));

    opentelemetry::logs::Provider::SetLoggerProvider(provider);
}

} // namespace

auto main(int argc, char* argv[]) -> int
{
    if (argc > 1) {
        initOpenTelemetry(argv[1]);
    } else {
        initOpenTelemetry("http://localhost:4318/v1/logs");
    }

    logrin::LogFactory::Init(
        logrin::LogLevel::Trace, {}, { std::make_shared<logrin::sinks::OpenTelemetry>("logrin/test") });

    auto logger = logrin::LogFactory::Get("test");
    logger.Info("Hello, world!").With("disk", "/dev/sda1").With("num", 32).With("bool", true).With("double", 1.234);

    logrin::LogFactory::Shutdown();
}
