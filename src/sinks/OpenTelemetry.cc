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

#include <logrin/Sinks/OpenTelemetry.h>

#if LOGRIN_FEATURE(OPENTELEMETRY)

#include "opentelemetry/logs/provider.h"

using logrin::sinks::OpenTelemetry;

using violet::SharedPtr;
using violet::Vec;

using opentelemetry::common::SystemTimestamp;
using opentelemetry::logs::Logger;
using opentelemetry::logs::Provider;
using opentelemetry::logs::Severity;

OpenTelemetry::OpenTelemetry(violet::Str tag, Options opts) noexcept
    : OpenTelemetry(Provider::GetLoggerProvider()->GetLogger(tag), opts)
{
}

OpenTelemetry::OpenTelemetry(SharedPtr<Logger> otel, Options opts) noexcept
    : n_otel(VIOLET_MOVE(otel))
    , n_batcher(
          [this](Vec<LogRecord>&& batched) -> void {
              auto batch = VIOLET_MOVE(batched);
              for (auto& record: batch) {
                  this->n_otel->EmitLogRecord(this->logRecordToOpenTelemetry(record));
              }
          },
          opts)
{
}

void OpenTelemetry::Enqueue(const LogRecord& record)
{
    this->n_batcher.Push(record);
}

void OpenTelemetry::Flush() noexcept
{
    this->n_batcher.Flush();
}

/*
void OpenTelemetry::Enqueue(const LogRecord& record) { this->n_batcher.Push(record); }
void OpenTelemetry::Flush() noexcept { this->n_batcher.Flush(); }
// destructor, workerLoop, n_worker/n_cv/n_mux/n_queue/n_running/n_processing all gone

*/

auto OpenTelemetry::logRecordToOpenTelemetry(const LogRecord& record) noexcept
    -> violet::UniquePtr<opentelemetry::logs::LogRecord>
{
    auto log = this->n_otel->CreateLogRecord();
    constexpr auto logLevelToSeverity = [](LogLevel level) -> Severity {
        switch (level) {
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

        default:
            VIOLET_UNREACHABLE();
        }
    };

    log->SetObservedTimestamp(SystemTimestamp(record.Timestamp));
    log->SetSeverity(logLevelToSeverity(record.Level));
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
            violet::UInt64 num = value.As<violet::UInt64>().UnwrapUnchecked(
                violet::Unsafe("we are already in the case where this should never fail"));

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
        if (value.Is<violet::experimental::Mono>()) {
            continue;
        }

        log->SetAttribute(key, convertLogrinAttributeValue(value));
    }

    // TODO(@auguwu/Noel): should we provide `source.location` and `log.name` in attributes?
    return log;
}

#endif
