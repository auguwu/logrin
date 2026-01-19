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

#include <chrono>
#include <violet/Violet.h>

namespace logrin {

/// TimePoint is a point in time with millisecond precision.
using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

/// The severity level of a log record.
enum struct LogLevel : violet::UInt8 {
    Trace, ///< fine-grained diagnostic information
    Debug, ///< debug-level messages that are useful during development
    Info, ///< informational message describing normal operations.
    Warning, ///< recoverable issues or unexpected situations
    Error, ///< errors that are prevented an operation from completing
    Fatal ///< critical errors that typically precede process termination.
};

/// A dynamically, typed attribute value.
///
/// AttributeValue represents a small, closed set of value types that can be attached
/// to log records as structured metadata. It is designed to be type-safe, allocation-aware,
/// and RTTI-free.
struct VIOLET_API AttributeValue final {
    /// Constructs an empty attribute value.
    ///
    /// The resulting value holds no data (`std::monostate`).
    constexpr VIOLET_IMPLICIT AttributeValue() noexcept = default;

    /// Constructs a boolean attribute value.
    constexpr VIOLET_IMPLICIT AttributeValue(bool value) noexcept
        : n_data(value)
    {
    }

    /// Constructs a signed 64-bit integer attribute value.
    constexpr VIOLET_IMPLICIT AttributeValue(violet::Int64 value) noexcept
        : n_data(value)
    {
    }

    /// Constructs an unsigned 64-bit integer attribute value.
    constexpr VIOLET_IMPLICIT AttributeValue(violet::UInt64 value) noexcept
        : n_data(value)
    {
    }

    /// Constructs a floating-point attribute value.
    constexpr VIOLET_IMPLICIT AttributeValue(double value) noexcept
        : n_data(value)
    {
    }

    /// Constructs a string attribute value.
    ///
    /// Ownership of the string is taken by value.
    template<std::convertible_to<violet::String> T>
    constexpr VIOLET_IMPLICIT AttributeValue(T value) noexcept
        : n_data(value)
    {
    }

    /// Returns **true** if this attribute value holds type `T`.
    template<typename T>
    [[nodiscard]] constexpr auto Is() const noexcept -> bool
    {
        return std::holds_alternative<T>(this->n_data);
    }

    /// Returns a pointer to the attribute's value if it is represented by type `T`
    template<typename T>
    constexpr auto As() const noexcept -> const T*
    {
        return std::get_if<T>(&this->n_data);
    }

private:
    using value_t = std::variant<std::monostate, bool, violet::Int64, violet::UInt64, double, violet::String>;

    value_t n_data;
};

/// LogRecord is a structured log event.
///
/// This struct represents a single logging event produced by a `Logger` and
/// delivered to one or more sinks. It contains both unstructured text and
/// structured key-value fields for rich diagnostics.
struct VIOLET_API LogRecord final {
    TimePoint Timestamp; ///< time point at which this log record was created
    LogLevel Level; ///< severity of the log event.
    violet::Str Message; ///< primary log message
    violet::UnorderedMap<violet::String, AttributeValue> Fields; ///< structured key-value pairs attached to the record
    violet::Str Logger; ///< name of the logger that emitted this record
    std::source_location Location; ///< source location where this log was emitted

    /// Constructs a [`LogRecord`] that sets the timestamp
    static auto Now(LogLevel level, violet::Str message,
        const std::source_location& loc = std::source_location::current()) noexcept -> LogRecord;

    /// Sets the `logger` field.
    /// @param name the logger's name
    auto WithLogger(violet::Str name) noexcept -> LogRecord&;

    /// Emplace a new field into this log record.
    /// @param name field name
    /// @param value field value
    auto With(violet::Str name, AttributeValue&& value) noexcept -> LogRecord&
    {
        this->Fields.emplace(std::make_pair(violet::String(name), VIOLET_MOVE(value)));
        return *this;
    }
};

} // namespace logrin

VIOLET_TO_STRING(const logrin::LogLevel&, level, {
    switch (level) {
    case logrin::LogLevel::Trace:
        return "trace";

    case logrin::LogLevel::Debug:
        return "debug";

    case logrin::LogLevel::Info:
        return "info";

    case logrin::LogLevel::Warning:
        return "warning";

    case logrin::LogLevel::Error:
        return "error";

    case logrin::LogLevel::Fatal:
        return "fatal";
    }
});

/**
 * @macro FIELD(name, value)
 *
 * This macro is meant to be inside of [`logrin::Logger::Log`] to make emplacing
 * an attribute field in a logger concise and easy.
 */
#define FIELD(name, value) ::std::make_pair<::violet::String, ::logrin::AttributeValue>(name, value)
