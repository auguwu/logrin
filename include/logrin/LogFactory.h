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

#include <logrin/Logger.h>
#include <violet/Violet.h>

#include <initializer_list>

namespace logrin {

struct Sink;
struct AsyncSink;

/// A factory-style way for constructing and managing loggers.
///
/// `logrin::LogFactory` provides a centralized way to obtain loggers by name,
/// configure logging sinks (both sync and async), and manage the global logging
/// lifecycle.
///
/// ## Example
/// ```cpp
/// #include <logrin/Sinks/Console/Formatter/Azaila.h>
/// #include <logrin/Sinks/Console.h>
/// #include <logrin/LogFactory.h>
///
/// using logrin::sinks::console::formatters::Azalia;
/// using logrin::sinks::Console;
/// using logrin::LogFactory;
///
/// LogFactory::Init(
///     logrin::LogLevel::Info,                  // global log level for all loggers
///     { std::make_shared<Console>(Azalia{}) }, // sync sinks
///     {}                                       // async sinks
/// );
///
/// auto log = LogFactory::Get("my::logger");
/// log.Info("Hello, world!");
/// ```
struct VIOLET_API LogFactory final {
    /// Retrieves a logger with the specified name.
    ///
    /// If the logger doesn't exist already, it'll be created and stored internally.
    /// If the log factory was not initialized, it'll return a dummy logger that doesn't log anything.
    ///
    /// @param name unique name of the logger to retrieve.
    /// @returns logger instance corresponding to the requested name.
    static auto Get(violet::Str name) noexcept -> Logger;

    /// Initializes the logging system.
    ///
    /// Configures the global log level, synchronous sinks, and asynchronous sinks.
    ///
    /// @param level global log level.
    /// @param sinks list of synchronous sinks for log output.
    /// @param asyncSinks list of asynchronous sinks for log output.
    static void Init(LogLevel level, std::initializer_list<violet::SharedPtr<Sink>> sinks = {},
        std::initializer_list<violet::SharedPtr<AsyncSink>> asyncSinks = {}) noexcept;

    /// Shuts down the logging system, flushing any pending asynchronous logs.
    static void Shutdown() noexcept;

private:
    VIOLET_EXPLICIT LogFactory(LogLevel level, std::initializer_list<violet::SharedPtr<Sink>> sinks,
        std::initializer_list<violet::SharedPtr<AsyncSink>> asyncSinks = {}) noexcept;

    LogLevel n_level;
    violet::Vec<violet::SharedPtr<Sink>> n_sinks;
    violet::Vec<violet::SharedPtr<AsyncSink>> n_asyncSinks;
    violet::UnorderedMap<violet::String, Logger> n_loggers;
};

} // namespace logrin
