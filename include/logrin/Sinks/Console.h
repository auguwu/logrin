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

#include <logrin/Sink.h>
#include <logrin/Sinks/Console/Formatter.h>
#include <violet/IO/Descriptor.h>
#include <violet/Violet.h>

namespace logrin::sinks {

/// Logging sink that writes formatted log records to a console stream.
///
/// **Console** is a concrete, synchronous [`Sink`] implementation that emits
/// log records to a process' standard input or error. Output formatting is
/// delegated to a userland [`console::Formatter`] implementation that allows
/// customization how log records are rendered before being written.
///
/// By default, the console writes JSON-style records to [`Stream::Stdout`].
///
/// ## Thread Safety
/// Emission is internally synchronized via a Mutex. Concurrent calls to [`Console::Emit`]
/// and [`Console::Flush`] are safe, through ordering between is not guaranteed.
///
/// ## Example
/// ```cpp
/// #include <logrin/Logger.h>
/// #include <logrin/Sinks/Console.h>
/// #include <logrin/Sinks/Console/Formatter/Azalia.h>
///
/// auto sink = new logrin::sinks::Console();
/// sink->WithFormatter<logrin::sinks::console::formatters::Azalia>();
///
/// logrin::Logger logger("main", logrin::LogLevel::Trace);
/// logger.AddSink(sink);
///
/// logger.Info("Hello, world!");
/// ```
struct VIOLET_API Console final: public Sink {
    /// What console stream to write formatted logs to.
    enum struct Stream : violet::UInt8 {
        Stdout, ///< process' standard input
        Stderr ///< process' standard error
    };

    /// Creates a new console sink with no formatter attached.
    /// @param stream The stream to use.
    VIOLET_IMPLICIT Console(Stream stream = Stream::Stdout) noexcept
        : n_stream(stream)
    {
    }

    /// Creates a new console sink with a pre-defined formatter attached to it.
    /// @param args the constructor arguments for `T`.
    template<typename T, typename... Args>
        requires(std::is_base_of_v<console::Formatter, T> && std::is_constructible_v<T, Args...>)
    VIOLET_EXPLICIT Console(Stream stream, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : n_formatter(std::make_shared<T>(VIOLET_FWD(Args, args)...))
        , n_stream(stream)
    {
    }

    /// Replaces the current formatter for this sink.
    /// @param formatter the formatter to replace
    template<typename T>
        requires(std::is_base_of_v<console::Formatter, T>)
    auto WithFormatter(T&& formatter) noexcept -> Console&
    {
        this->n_formatter = std::make_shared<T>(VIOLET_FWD(T, formatter));
        return *this;
    }

    /// Replaces the sink's stream to something else.
    /// @param stream The stream to replace
    auto WithStream(Stream stream) noexcept -> Console&;

    /// Replaces the current formatter for this sink to `T`.
    /// @param args constructor arguments of `T`.
    template<typename T, typename... Args>
        requires(std::is_base_of_v<console::Formatter, T> && std::is_constructible_v<T, Args...>)
    auto WithFormatter(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) -> Console&
    {
        this->n_formatter = std::make_shared<T>(VIOLET_FWD(Args, args)...);
        return *this;
    }

    /// @see logrin::Sink::Emit(const logrin::LogRecord&)
    void Emit(const LogRecord& record) override;

    /// @see logrin::Sink::Flush()
    void Flush() noexcept override;

private:
    violet::SharedPtr<console::Formatter> n_formatter;
    violet::io::FileDescriptor n_descriptor;
    Stream n_stream = Stream::Stdout;

    mutable violet::Mutex n_mux;
};

} // namespace logrin::sinks
