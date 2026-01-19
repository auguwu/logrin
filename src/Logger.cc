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

#include <logrin/AsyncSink.h>
#include <logrin/Logger.h>
#include <logrin/Sink.h>

using logrin::AsyncSink;
using logrin::LogEntry;
using logrin::Logger;
using logrin::Sink;

using violet::Str;

LogEntry::LogEntry(Logger* logger, LogRecord record, bool emit) noexcept
    : n_record(VIOLET_MOVE(record))
    , n_parent(logger)
    , n_emit(emit)
{
}

LogEntry::~LogEntry()
{
    if (this->n_emit && this->n_parent != nullptr) {
        for (const auto& sink: this->n_parent->Sinks()) {
            sink->Emit(this->n_record);
        }

        for (const auto& sink: this->n_parent->AsyncSinks()) {
            sink->Enqueue(this->n_record);
        }

        this->n_emit = false;
        this->n_parent = nullptr;
    }
}

Logger::Logger(
    Str name, LogLevel level, std::initializer_list<Sink*> sinks, std::initializer_list<AsyncSink*> asyncSinks) noexcept
    : n_level(level)
    , n_name(name)
{
    for (const auto& sink: sinks) {
        this->n_sinks.emplace_back(sink);
    }

    for (const auto& asyncSink: asyncSinks) {
        this->n_asyncSinks.emplace_back(asyncSink);
    }
}

Logger::~Logger() noexcept
{
    this->flush();
}

auto Logger::WithName(Str name) noexcept -> Logger&
{
    this->n_name = name;
    return *this;
}

auto Logger::Sinks() const noexcept -> violet::Span<const violet::SharedPtr<Sink>>
{
    return this->n_sinks;
}

auto Logger::AsyncSinks() const noexcept -> violet::Span<const violet::SharedPtr<AsyncSink>>
{
    return this->n_asyncSinks;
}

auto Logger::Name() const noexcept -> violet::Str
{
    return this->n_name;
}

auto Logger::Log(LogRecord record) noexcept -> LogEntry
{
    record.Logger = this->Name();
    return LogEntry(this, record);
}

void Logger::flush() noexcept
{
    for (const auto& sink: this->n_sinks)
        sink->Flush();

    for (const auto& sink: this->n_asyncSinks)
        sink->Flush();
}
