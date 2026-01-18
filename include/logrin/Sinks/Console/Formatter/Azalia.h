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
#include <logrin/Sinks/Console/Formatter.h>
#include <logrin/bits/Macros.h>
#include <violet/Support/Terminal.h>
#include <violet/Violet.h>

namespace logrin::sinks::console::formatters {

/// A console formatter that represents the output of Noelware's `azalia_log` Rust crate.
///
/// See:
/// <https://github.com/Noelware/azalia/blob/61ab79859a014cd7c2c45bbdd9d4925a6a896bc0/crates/log/src/writers/default.rs>
struct LOGRIN_API Azalia final: public Formatter {
    VIOLET_IMPLICIT Azalia() noexcept = default;

    auto WithTimestampFormat(violet::Str fmt) noexcept -> Azalia&;
    auto WithPrintTimestamp(bool yes = true) noexcept -> Azalia&;
    auto WithPrintLogger(bool yes = true) noexcept -> Azalia&;
    auto WithPrintLevel(bool yes = true) noexcept -> Azalia&;
    auto WithColors(bool yes = true) noexcept -> Azalia&;

    [[nodiscard]] auto Format(const LogRecord& record) const noexcept -> violet::String override;

private:
    constexpr static const violet::terminal::RGB kTrace = violet::terminal::RGB(163, 182, 138); // #A3B68A
    constexpr static const violet::terminal::RGB kDebug = violet::terminal::RGB(148, 224, 232); // #94E0E8
    constexpr static const violet::terminal::RGB kError = violet::terminal::RGB(153, 75, 104); // #994B68
    constexpr static const violet::terminal::RGB kWarn = violet::terminal::RGB(243, 243, 134); // #F3F386
    constexpr static const violet::terminal::RGB kInfo = violet::terminal::RGB(178, 157, 243); // #B29DF3
    constexpr static const violet::terminal::RGB kGray = violet::terminal::RGB(34, 34, 34); // rgb(34,34,34)

    violet::String n_timestampFormat = "%B %d, %Y - %I:%M:%S %p"; // month day, year - hour:minute:second AM/PM
    bool n_printTimestamp = true;
    bool n_printLogger = true;
    bool n_printLevel = true;
    bool n_colors = true;

    [[nodiscard]] auto printTimestamp(TimePoint ts) const noexcept -> violet::String;
    auto printLogger(std::ostream& os, violet::String logger) const noexcept -> violet::String;
    void printLevel(std::ostream& os, LogLevel level) const noexcept;
    void printAttributes(
        std::ostream& os, const violet::UnorderedMap<violet::String, AttributeValue>& attrs) const noexcept;
};

} // namespace logrin::sinks::console::formatters
