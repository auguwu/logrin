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

#include <violet/Violet.h>

namespace logrin {
struct LogRecord;
}

namespace logrin::sinks::console {

/// A log record formatter for console sinks.
///
/// Formatter defines a interface for converting [`LogRecord`]s into a string suitable
/// for console output. Concrete implementations decide the visual style, colourization,
/// and structure of log messages.
///
/// ## Example
/// ```
/// #include <logrin/Sinks/Console/Formatter.h>
/// #include <logrin/Sinks/Console.h>
/// #include <logrin/LogRecord.h>
/// #include <logrin/Logger.h>
///
/// using namespace logrin;
/// using namespace logrin::sinks;
/// using namespace logrin::sinks::console;
///
/// struct MyFormatter final: public Formatter {
///     [[nodiscard]] auto Format(const LogRecord& record) const noexcept -> violet::String {
///         return std::format("[{}] {}", violet::ToString(record.Level), record.Message);
///     }
/// };
///
/// Logger logger("main", LogLevel::Trace);
/// logger.AddSink<Console>(MyFormatter());
///
/// logger.Info("Hello, world!");
/// /// => [info] Hello, world!
/// ```
struct VIOLET_API Formatter {
    /// Virtual destructor.
    ///
    /// Allows derived formatters to clean up properly when deleted
    /// via a `Formatter*`.
    virtual ~Formatter() = default;

    /// Converts a log record into a string for console output.
    /// @param record the record to format
    [[nodiscard]] virtual auto Format(const LogRecord& record) const noexcept -> violet::String = 0;
};

} // namespace logrin::sinks::console
