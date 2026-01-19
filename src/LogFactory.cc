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
#include <logrin/LogFactory.h>
#include <logrin/Logger.h>
#include <logrin/Sink.h>

using logrin::AsyncSink;
using logrin::LogFactory;
using logrin::Sink;
using violet::Str;

static LogFactory* instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

LogFactory::LogFactory(
    LogLevel level, std::initializer_list<Sink*> sinks, std::initializer_list<AsyncSink*> asyncSinks) noexcept
    : n_level(level)
{
    for (const auto& sink: sinks) {
        this->n_sinks.emplace_back(sink);
    }

    for (const auto& asyncSink: asyncSinks) {
        this->n_asyncSinks.emplace_back(asyncSink);
    }
}

auto LogFactory::Get(Str name) noexcept -> Logger
{
    VIOLET_DEBUG_ASSERT(instance != nullptr, "factory was not initialized with `logrin::LogFactory::Init`");
    if (auto it = instance->n_loggers.find(violet::String(name)); it != instance->n_loggers.end()) {
        return it->second;
    }

    Logger log(name, instance->n_level);
    for (const auto& sink: instance->n_sinks) {
        log.n_sinks.emplace_back(sink);
    }

    for (const auto& sink: instance->n_asyncSinks) {
        log.n_asyncSinks.emplace_back(sink);
    }

    instance->n_loggers.emplace(std::make_pair(violet::String(name), log));
    return log;
}

void LogFactory::Init(
    LogLevel level, std::initializer_list<Sink*> sinks, std::initializer_list<AsyncSink*> asyncSinks) noexcept
{
    if (instance != nullptr) {
        return;
    }

    instance = new LogFactory(level, sinks, asyncSinks);
}

void LogFactory::Shutdown() noexcept
{
    if (instance == nullptr) {
        return;
    }

    for (auto& [name, log]: instance->n_loggers) {
        log.flush();
    }

    delete instance;
}
