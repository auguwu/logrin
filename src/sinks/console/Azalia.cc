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

#include <logrin/Sinks/Console/Formatter/Azalia.h>

using logrin::AttributeValue;
using logrin::sinks::console::formatters::Azalia;
using violet::terminal::Style;
using violet::terminal::Styled;

auto Azalia::WithTimestampFormat(violet::Str fmt) noexcept -> Azalia&
{
    this->n_timestampFormat = fmt;
    return *this;
}

auto Azalia::WithPrintTimestamp(bool yes) noexcept -> Azalia&
{
    this->n_printTimestamp = yes;
    return *this;
}

auto Azalia::WithPrintLevel(bool yes) noexcept -> Azalia&
{
    this->n_printLevel = yes;
    return *this;
}

auto Azalia::WithPrintLogger(bool yes) noexcept -> Azalia&
{
    this->n_printLogger = yes;
    return *this;
}

auto Azalia::WithColors(bool yes) noexcept -> Azalia&
{
    this->n_colors = yes;
    return *this;
}

auto Azalia::Format(const LogRecord& record) const noexcept -> violet::String
{
    std::stringstream os;
    if (this->n_printTimestamp) {
        os << this->printTimestamp(record.Timestamp) << ' ';
    }

    if (this->n_printLevel) {
        this->printLevel(os, record.Level);
        os << ' ';
    }

    // if (this->n_printLogger) {
    //     os << this->printLogger(os, violet::String(record.Logger));
    // }

    os << record.Message;

    // if (!record.Fields.empty()) {
    //     os << "    " << this->printAttributes(os, record.Fields);
    // }

    return os.str();
}

auto Azalia::printTimestamp(TimePoint ts) const noexcept -> violet::String
{
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::seconds>(ts));
    auto [lBracket, rBracket]
        = std::make_pair(this->n_colors ? Styled<violet::String>("[", Style::RGB(134, 134, 134)).Paint() : "[",
            this->n_colors ? Styled<violet::String>("]", Style::RGB(134, 134, 134)).Paint() : "]");

    std::tm tm = *std::localtime(&time);

    char buf[64];
    if (std::strftime(buf, sizeof(buf), this->n_timestampFormat.c_str(), &tm) == 0) {
        buf[0] = '\0';
    }

    return std::format(
        "{}{}{}", lBracket, Styled<violet::String>(violet::String(buf), Style::RGB(134, 134, 134)).Paint(), rBracket);
}

void Azalia::printLevel(std::ostream& os, LogLevel level) const noexcept {}

void Azalia::printAttributes(
    std::ostream& os, const violet::UnorderedMap<violet::String, AttributeValue>& attrs) const noexcept
{
}
