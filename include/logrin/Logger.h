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

#include <type_traits>

namespace logrin {

struct Sink;
struct AsyncSink;
struct Logger;

/// Represents a single log entry.
///
/// A `LogEntry` is a handle returned by `Logger` when logging a message.
/// It allows attaching additional key/value attributes before the entry is emitted.
struct VIOLET_API LogEntry final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(LogEntry);
    VIOLET_DISALLOW_COPY_AND_MOVE(LogEntry);

    ~LogEntry();

    /// Attaches an additional attribute to the log entry.
    ///
    /// @tparam T type that is constructible as an `AttributeValue`.
    /// @param name name/key of the attribute.
    /// @param value value to associate with the attribute.
    /// @returns a reference to `*this` to allow chaining.
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

/// A logger that produces log entries and manages sinks.
///
/// Provides convenience methods for logging at different levels (Trace, Debug, Info, etc.).
/// Supports adding synchronous and asynchronous sinks.
struct VIOLET_API Logger final {
    VIOLET_DISALLOW_CONSTRUCTOR(Logger);
    ~Logger() noexcept;

    /// Constructs a logger with a name, log level, and optional sinks.
    ///
    /// @param name unique name of the logger.
    /// @param level minimum log level.
    /// @param sinks optional synchronous sinks.
    /// @param asyncSinks optional asynchronous sinks.
    VIOLET_IMPLICIT Logger(violet::Str name, LogLevel level, std::initializer_list<Sink*> sinks = {},
        std::initializer_list<AsyncSink*> asyncSinks = {}) noexcept;

    /// Returns whether logging is enabled for a given level.
    [[nodiscard]] constexpr auto Enabled(LogLevel level) const noexcept -> bool
    {
        if (level == LogLevel::Off) {
            return false;
        }

        return level >= this->n_level;
    }

    /// Sets or changes the name of this logger.
    auto WithName(violet::Str name) noexcept -> Logger&;

    /// Adds a synchronous sink to the logger.
    template<typename SinkT>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(SinkT* sink) noexcept -> Logger&
    {
        this->n_sinks.emplace_back(sink);
        return *this;
    }

    /// Constructs and adds a new synchronous sink in-place.
    template<typename SinkT, typename... Args>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(Args&&... args) noexcept(std::is_constructible_v<SinkT, Args...>) -> Logger&
    {
        this->n_sinks.emplace_back(std::make_shared<SinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    /// Adds an asynchronous sink to the logger.
    template<typename AsyncSinkT>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(AsyncSinkT* sink) noexcept -> Logger&
    {
        this->n_asyncSinks.emplace_back(sink);
        return *this;
    }

    /// Constructs and adds a new asynchronous sink in-place.
    template<typename AsyncSinkT, typename... Args>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(Args&&... args) noexcept(std::is_constructible_v<AsyncSinkT, Args...>) -> Logger&
    {
        this->n_asyncSinks.emplace_back(std::make_shared<AsyncSinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    /// Returns a view of the synchronous sinks.
    [[nodiscard]] auto Sinks() const noexcept -> violet::Span<const violet::SharedPtr<Sink>>;

    /// Returns a view of the asynchronous sinks.
    [[nodiscard]] auto AsyncSinks() const noexcept -> violet::Span<const violet::SharedPtr<AsyncSink>>;

    /// Returns the logger's name.
    [[nodiscard]] auto Name() const noexcept -> violet::Str;

    /// Logs a new record and allows manipulation of the entry itself.
    auto Log(LogRecord record) noexcept -> LogEntry;

    /// Logs a message at a specified log level.
    ///
    /// @tparam Msg type convertible to `violet::Str`.
    /// @param level log level for the message.
    /// @param message message to log.
    /// @param loc source location, defaults to the caller.
    /// @returns a `LogEntry` for further attribute attachment.
    template<std::convertible_to<violet::Str> Msg>
    auto Log(LogLevel level, Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept
        -> LogEntry
    {
        if (!this->Enabled(level)) {
            return LogEntry(nullptr, LogRecord{}, false);
        }

        return this->Log(LogRecord::Now(level, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Trace-level message.
    template<std::convertible_to<violet::Str> Msg>
    auto Trace(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Trace, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Debug-level message.
    template<std::convertible_to<violet::Str> Msg>
    auto Debug(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Debug, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Info-level message.
    template<std::convertible_to<violet::Str> Msg>
    auto Info(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Info, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Warning-level message.
    template<std::convertible_to<violet::Str> Msg>
    auto Warn(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Warning, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Error-level message.
    template<std::convertible_to<violet::Str> Msg>
    auto Error(Msg&& message, const std::source_location& loc = std::source_location::current()) noexcept -> LogEntry
    {
        return this->Log(LogRecord::Now(LogLevel::Error, VIOLET_FWD(Msg, message), loc));
    }

    /// Convenience method: logs a Fatal-level message.
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
