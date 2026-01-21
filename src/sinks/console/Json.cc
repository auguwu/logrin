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

#include <logrin/LogRecord.h>
#include <logrin/Sinks/Console/Formatter/Json.h>
#include <violet/Support/Demangle.h>

using logrin::sinks::console::formatters::Json;

using violet::Int32;

auto Json::WithIndentation(Int32 indent) noexcept -> Json&
{
    this->n_indent = indent;
    return *this;
}

auto Json::WithPretty(bool yes) noexcept -> Json&
{
    this->n_pretty = yes;
    return *this;
}

auto Json::Format(const LogRecord& record) const noexcept -> violet::String
{
    auto value = intoJson(record);
    return std::format("{}\n", value.dump(this->n_pretty ? this->n_indent : -1, ' ', true));
}

auto Json::intoJson(const LogRecord& record) noexcept -> nlohmann::json
{
    auto tt = std::chrono::system_clock::to_time_t(record.Timestamp);
    auto gmt = *std::gmtime(&tt);

    char buf[64];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &gmt) == 0) {
        buf[0] = '\0';
    }

    nlohmann::json value;
    value["@timestamp"] = violet::String(buf);
    value["message"] = record.Message;

    nlohmann::json log;
    log["logger"] = record.Logger;
    log["level"] = violet::ToString(record.Level);

    value["log"] = log;

    nlohmann::json src;
    src["file"] = record.Location.file_name();
    src["line"] = record.Location.line();
    src["column"] = record.Location.column();
    src["function"] = violet::util::DemangleCXXName(record.Location.function_name());
    value["source"] = src;

    nlohmann::json fields;
    for (const auto& [name, field]: record.Fields) {
        nlohmann::json fieldValue;
        if (field.Is<bool>()) {
            fieldValue = *field.As<bool>();
        } else if (field.Is<violet::Int64>()) {
            fieldValue = *field.As<violet::Int64>();
        } else if (field.Is<violet::UInt64>()) {
            fieldValue = *field.As<violet::UInt64>();
        } else if (field.Is<double>()) {
            fieldValue = *field.As<double>();
        } else if (field.Is<violet::String>()) {
            fieldValue = *field.As<violet::String>();
        }

        if (!fieldValue.is_null()) {
            fields[name] = fieldValue;
        }
    }

    if (!fields.is_null()) {
        value["attributes"] = fields;
    }

    return value;
}
