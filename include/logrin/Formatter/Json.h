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

#include <logrin/Formatter.h>

namespace logrin::formatter {

/// A log record formatter that serializes log records as JSON.
///
/// `Json` implements the `Formatter` interface, producing JSON-encoded output
/// from a `LogRecord`. By default, output is compact (single-line); enable
/// `Pretty` for human-readable, indented output.
///
/// ## Example
/// ```cpp
/// #include <logrin/Formatter/Json.h>
/// #include <logrin/Sinks/Console.h>
///
/// // Compact JSON (default)
/// auto formatter = logrin::formatter::Json();
///
/// // Pretty-printed with default 4-space indentation
/// auto formatter = logrin::formatter::Json().Pretty();
/// ```
struct LOGRIN_API Json final: public Formatter {
    struct Config final {
        /// Whether to emit pretty-printed (indented, multi-line) JSON.
        bool Pretty = false;

        /// The number of spaces per indentation level when `Pretty` is enabled.
        ///
        /// Has no effect when `Pretty` is `false`. Defaults to `4`.
        violet::Int32 Indentation = 4;
    };

    VIOLET_IMPLICIT Json() noexcept = default;

    /// Constructs a `Json` formatter from the given configuration.
    VIOLET_IMPLICIT Json(Config config) noexcept
        : n_config(config)
    {
    }

    /// Sets the indentation width used when pretty-printing.
    ///
    /// This is a builder-style method that returns a reference to `*this`,
    /// allowing it to be chained with other configuration methods.
    template<std::convertible_to<violet::Int32> Int>
    auto WithIndentation(Int&& indent) noexcept -> Json&
    {
        this->n_config.Indentation = VIOLET_FWD(Int, indent);
        return *this;
    }

    /// Enables or disables pretty-printed output.
    auto Pretty(bool yes = true) noexcept -> Json&;

    /// Enables or disables pretty-printed output.
    ///
    /// ## Deprecated (since 26.05)
    /// Use [`Json::Pretty(bool)`] instead.
    LOGRIN_DEPRECATED("26.05", "function will be replaced with `Json::Pretty`")
    auto WithPretty(bool yes = true) noexcept -> Json&;

    /// @see logrin::Formatter::Format(const LogRecord&)
    [[nodiscard]] auto Format(const LogRecord& record) const -> violet::String override;

private:
    Config n_config;
};

} // namespace logrin::formatter
