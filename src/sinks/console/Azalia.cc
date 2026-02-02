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

#include <algorithm>

using logrin::AttributeValue;
using logrin::LogLevel;
using logrin::sinks::console::formatters::Azalia;

using violet::String;
using violet::terminal::Style;
using violet::terminal::Styled;

namespace {

constexpr const violet::terminal::Style kTrace = violet::terminal::Style::RGB(163, 182, 138); // #A3B68A
constexpr const violet::terminal::Style kDebug = violet::terminal::Style::RGB(148, 224, 232); // #94E0E8
constexpr const violet::terminal::Style kError = violet::terminal::Style::RGB(255, 82, 82); // #ff5252
constexpr const violet::terminal::Style kWarn = violet::terminal::Style::RGB(243, 243, 134); // #F3F386
constexpr const violet::terminal::Style kInfo = violet::terminal::Style::RGB(178, 157, 243); // #B29DF3

constexpr auto levelToStyle(LogLevel level) -> Style
{
    switch (level) {
    case LogLevel::Trace:
        return kTrace;

    case LogLevel::Debug:
        return kDebug;

    case LogLevel::Info:
        return kInfo;

    case LogLevel::Warning:
        return kWarn;

    case LogLevel::Error:
        return kError;

    case LogLevel::Fatal: {
        auto error = kError;
        return error.Bold();
    } break;
    }
}

constexpr void writeAttrValue(std::ostream& os, const AttributeValue& value) noexcept
{
    if (value.Is<std::monostate>()) {
        os << "{null}";
        return;
    }

    if (value.Is<bool>()) {
        os << std::boolalpha << *value.As<bool>();
        return;
    }

    if (value.Is<violet::Int64>()) {
        os << *value.As<violet::Int64>();
        return;
    }

    if (value.Is<violet::UInt64>()) {
        os << *value.As<violet::UInt64>();
        return;
    }

    if (value.Is<double>()) {
        os << *value.As<double>();
        return;
    }

    if (value.Is<String>()) {
        os << '"' << *value.As<String>() << '"';
        return;
    }

    VIOLET_UNREACHABLE();
}

} // namespace

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

    if (this->n_printLogger) {
        if (this->n_colors) {
            os << Styled<String>(std::format("[{}]", record.Logger), kGray).Paint();
        } else {
            os << '[' << record.Logger << ']';
        }

        os << ' ';
    }

    os << record.Message;

    if (!record.Fields.empty()) {
        os << "    ";
        this->printAttributes(os, record.Fields);
    }

    os << '\n';
    return os.str();
}

auto Azalia::printTimestamp(TimePoint ts) const noexcept -> violet::String
{
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::seconds>(ts));
    std::tm tm = *std::localtime(&time);

    char buf[64];
    if (std::strftime(buf, sizeof(buf), this->n_timestampFormat.c_str(), &tm) == 0) {
        buf[0] = '\0';
    }

    if (!this->n_colors) {
        return std::format("[{}]", buf);
    }

    return Styled<String>(std::format("[{}]", buf), kGray).Paint();
}

void Azalia::printLevel(std::ostream& os, LogLevel level) const noexcept
{
    auto levelStr = violet::ToString(level);
    std::ranges::transform(levelStr, levelStr.begin(), [](unsigned char ch) -> int { return std::toupper(ch); });

    if (this->n_colors) {
        os << Styled<String>(levelStr, levelToStyle(level)).Paint();
    } else {
        os << levelStr;
    }
}

void Azalia::printAttributes(
    std::ostream& os, const violet::UnorderedMap<violet::String, AttributeValue>& attrs) const noexcept
{
    for (const auto& [name, field]: attrs) {
        if (!this->n_colors) {
            os << '[' << name << '=';
            writeAttrValue(os, field);
            os << "] ";
        } else {
            std::stringstream oss;
            writeAttrValue(oss, field);

            constexpr auto gray = Style::RGB<34, 34, 34>().Bold().Italic();
            os << Styled<String>(std::format("[{}={}] ", name, oss.str()), gray).Paint();
        }
    }
}
