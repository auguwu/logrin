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

#include "opentelemetry/logs/provider.h"

#include <logrin/Sinks/OpenTelemetry.h>

using logrin::sinks::OpenTelemetry;

using violet::SharedPtr;

using opentelemetry::common::SystemTimestamp;
using opentelemetry::logs::Logger;
using opentelemetry::logs::Provider;
using opentelemetry::logs::Severity;

OpenTelemetry::OpenTelemetry(violet::Str tag) noexcept
    : OpenTelemetry(Provider::GetLoggerProvider()->GetLogger(tag))
{
}

OpenTelemetry::OpenTelemetry(SharedPtr<Logger> otel) noexcept
    : n_running(true)
    , n_otel(VIOLET_MOVE(otel))
{
    this->n_worker = std::thread([this]() -> void { this->workerLoop(); });
}

OpenTelemetry::~OpenTelemetry()
{
    this->n_running = false;
    this->n_cv.notify_all();

    if (this->n_worker.joinable()) {
        this->n_worker.join();
    }
}

void OpenTelemetry::Enqueue(const LogRecord& record)
{
    {
        std::lock_guard lock(this->n_mux);
        this->n_queue.push(record);
    }

    this->n_cv.notify_one();
}

void OpenTelemetry::Flush() noexcept
{
    std::unique_lock lock(this->n_mux);
    this->n_cv.wait(lock, [this]() -> bool { return this->n_queue.empty(); });
}

void OpenTelemetry::workerLoop()
{
    while (this->n_running || !this->n_queue.empty()) {
        LogRecord record;
        {
            std::unique_lock lock(this->n_mux);
            this->n_cv.wait(lock, [this]() -> bool { return !this->n_queue.empty() || !this->n_running; });

            if (!this->n_queue.empty()) {
                record = this->n_queue.front();
                this->n_queue.pop();
            } else {
                continue;
            }
        }

        auto otel_record = this->logRecordToOpenTelemetry(record);
        this->n_otel->EmitLogRecord(VIOLET_MOVE(otel_record));
    }
}

auto OpenTelemetry::logRecordToOpenTelemetry(const LogRecord& record) noexcept
    -> violet::UniquePtr<opentelemetry::logs::LogRecord>
{
    auto log = this->n_otel->CreateLogRecord();

    auto logRecordToSeverity = [&record]() -> Severity {
        VIOLET_DIAGNOSTIC_PUSH
#if defined(VIOLET_GCC) || defined(VIOLET_CLANG)
        VIOLET_DIAGNOSTIC_IGNORE("-Wswitch")
#endif
        switch (record.Level) {
        case LogLevel::Trace:
            return Severity::kTrace;

        case LogLevel::Debug:
            return Severity::kDebug;

        case LogLevel::Info:
            return Severity::kInfo;

        case LogLevel::Warning:
            return Severity::kWarn;

        case LogLevel::Error:
            return Severity::kError;

        case LogLevel::Fatal:
            return Severity::kFatal;
        }

        VIOLET_UNREACHABLE();
        VIOLET_DIAGNOSTIC_POP
    };

    log->SetObservedTimestamp(SystemTimestamp(record.Timestamp));
    log->SetSeverity(logRecordToSeverity());
    log->SetBody(record.Message);

    auto convertLogrinAttributeValue = [](const AttributeValue& value) -> opentelemetry::common::AttributeValue {
        if (value.Is<bool>()) {
            return *value.As<bool>();
        }

        if (value.Is<violet::Int64>()) {
            return *value.As<violet::Int64>();
        }

        if (value.Is<violet::UInt64>()) {
            // OpenTelemetry specification doesn't support uint64_t, so we'll do
            // a lossly conversion.
            violet::UInt64 num = *value.As<violet::UInt64>();
            VIOLET_ASSERT(num <= std::numeric_limits<violet::Int64>::max(),
                "lossly conversion not possible: reached max limit of int64");

            return static_cast<violet::Int64>(num);
        }

        if (value.Is<double>()) {
            return *value.As<double>();
        }

        if (value.Is<violet::String>()) {
            return *value.As<violet::String>();
        }

        VIOLET_UNREACHABLE();
    };

    for (const auto& [key, value]: record.Fields) {
        log->SetAttribute(key, convertLogrinAttributeValue(value));
    }

    // TODO(@auguwu/Noel): should we provide `source.location` and `log.name` in attributes?
    return log;
}
