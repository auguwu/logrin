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

#pragma once

#include "opentelemetry/logs/log_record.h"
#include "opentelemetry/logs/logger.h"

#include <logrin/AsyncSink.h>
#include <logrin/LogRecord.h>
#include <violet/Violet.h>

#include <queue>
#include <thread>

namespace logrin::sinks {

struct OpenTelemetry final: public AsyncSink {
    VIOLET_IMPLICIT OpenTelemetry(violet::Str tag) noexcept;
    VIOLET_IMPLICIT OpenTelemetry(violet::SharedPtr<opentelemetry::logs::Logger> otel) noexcept;

    ~OpenTelemetry() override;

    /// @see logrin::AsyncSink::Enqueue(const logrin::LogRecord&)
    void Enqueue(const LogRecord& record) override;

    /// @see logrin::AsyncSink::Flush()
    void Flush() noexcept override;

private:
    std::thread n_worker;
    std::mutex n_mux;
    std::condition_variable n_cv;
    std::queue<LogRecord> n_queue;
    std::atomic<bool> n_running;
    violet::SharedPtr<opentelemetry::logs::Logger> n_otel;

    void workerLoop();
    auto logRecordToOpenTelemetry(const LogRecord& record) noexcept
        -> violet::UniquePtr<opentelemetry::logs::LogRecord>;
};

} // namespace logrin::sinks
