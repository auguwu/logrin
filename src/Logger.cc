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
using logrin::Logger;
using logrin::Sink;
using violet::Str;

Logger::Logger(Str name) noexcept
    : n_name(name)
{
}

Logger::Logger(Str name, std::initializer_list<Sink*> sinks, std::initializer_list<AsyncSink*> asyncSinks) noexcept
    : Logger(name)
{
    for (const auto& sink: sinks) {
        this->n_sinks.emplace_back(sink);
    }

    for (const auto& asyncSink: asyncSinks) {
        this->n_asyncSinks.emplace_back(asyncSink);
    }
}

Logger::~Logger()
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

void Logger::Log(const LogRecord& record)
{
    for (const auto& sink: this->n_sinks) {
        sink->Emit(record);
    }

    for (const auto& sink: this->n_asyncSinks) {
        sink->Enqueue(record);
    }
}

void Logger::flush()
{
    for (const auto& sink: this->n_sinks)
        sink->Flush();

    for (const auto& sink: this->n_asyncSinks)
        sink->Flush();
}
