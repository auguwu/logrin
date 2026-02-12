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

#include <logrin/LogRecord.h>
#include <violet/Violet.h>

namespace logrin {

struct Sink;
struct AsyncSink;
struct Logger;

struct VIOLET_API LogEntry final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(LogEntry);
    VIOLET_DISALLOW_COPY_AND_MOVE(LogEntry);

    ~LogEntry();

    template<typename T>
        requires(std::is_constructible_v<AttributeValue, T>)
    auto With(violet::Str name, T value) noexcept -> LogEntry&
    {
        this->n_record = this->n_record.With(name, value);
        return *this;
    }

private:
    friend struct Logger;

    VIOLET_EXPLICIT LogEntry(Logger* logger, LogRecord record, bool emit = true) noexcept;

    LogRecord n_record; ///< the record that this entry is used for
    Logger* n_parent; ///< pointer to the parent logger
    bool n_emit = false;
};

struct VIOLET_API Logger final {
    VIOLET_DISALLOW_CONSTRUCTOR(Logger);
    ~Logger() noexcept;

    VIOLET_IMPLICIT Logger(violet::Str name, LogLevel level, std::initializer_list<Sink*> sinks = {},
        std::initializer_list<AsyncSink*> asyncSinks = {}) noexcept;

    [[nodiscard]] constexpr auto Enabled(LogLevel level) const noexcept -> bool
    {
        if (level == LogLevel::Off) {
            return false;
        }

        return level >= this->n_level;
    }

    auto WithName(violet::Str name) noexcept -> Logger&;

    template<typename SinkT>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(SinkT* sink) noexcept -> Logger&
    {
        this->n_sinks.emplace_back(sink);
        return *this;
    }

    template<typename SinkT, typename... Args>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(Args&&... args) noexcept(std::is_constructible_v<SinkT, Args...>) -> Logger&
    {
        this->n_sinks.emplace_back(std::make_shared<SinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    template<typename AsyncSinkT>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(AsyncSinkT* sink) noexcept -> Logger&
    {
        this->n_asyncSinks.emplace_back(sink);
        return *this;
    }

    template<typename AsyncSinkT, typename... Args>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(Args&&... args) noexcept(std::is_constructible_v<AsyncSinkT, Args...>) -> Logger&
    {
        this->n_asyncSinks.emplace_back(std::make_shared<AsyncSinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    [[nodiscard]] auto Sinks() const noexcept -> violet::Span<const violet::SharedPtr<Sink>>;
    [[nodiscard]] auto AsyncSinks() const noexcept -> violet::Span<const violet::SharedPtr<AsyncSink>>;
    [[nodiscard]] auto Name() const noexcept -> violet::Str;

    auto Log(LogRecord record) noexcept -> LogEntry;

    template<std::convertible_to<violet::Str> Msg>
    auto Log(LogLevel level, Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept
        -> LogEntry
    {
        if (!this->Enabled(level)) {
            return LogEntry(nullptr, LogRecord{}, false);
        }

        return this->Log(LogRecord::Now(level, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Trace(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Trace, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Debug(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Debug, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Info(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Info, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Warn(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Warning, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Error(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Error, VIOLET_FWD(Msg, message), loc));
    }

    template<std::convertible_to<violet::Str> Msg>
    auto Fatal(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Fatal, VIOLET_FWD(Msg, message), loc));
    }

private:
    friend struct LogFactory;

    LogLevel n_level;
    violet::String n_name;
    violet::Vec<violet::SharedPtr<Sink>> n_sinks;
    violet::Vec<violet::SharedPtr<AsyncSink>> n_asyncSinks;

    void flush() noexcept;
};

} // namespace logrin
