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

using logrin::LogLevel;
using logrin::LogRecord;
using violet::Str;

namespace {

auto now() noexcept -> logrin::TimePoint
{
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
}

} // namespace

auto LogRecord::Now(LogLevel level, Str message, violet::SourceLocation loc) noexcept -> LogRecord
{
    return { .Timestamp = now(), .Level = level, .Message = message, .Fields = {}, .Location = loc };
}

auto LogRecord::WithLogger(Str name) noexcept -> LogRecord&
{
    this->Logger = name;
    return *this;
}

auto LogRecord::AsJson() const noexcept -> nlohmann::json
{
    auto tt = std::chrono::system_clock::to_time_t(this->Timestamp);
    auto gmt = *std::gmtime(&tt);

    char buf[64];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &gmt) == 0) {
        buf[0] = '\0';
    }

    nlohmann::json value;
    value["@timestamp"] = violet::String(buf);
    value["message"] = this->Message;

    nlohmann::json log;
    log["logger"] = this->Logger;
    log["level"] = violet::ToString(this->Level);

    value["log"] = log;

    nlohmann::json src;
    if (!this->Location.File.empty()) {
        src["file"] = this->Location.File;
    }

    if (this->Location.Line > 0) {
        src["line"] = this->Location.Line;
    }

    if (this->Location.Column > 0) {
        src["column"] = this->Location.Column;
    }

    if (!this->Location.Function.empty()) {
        src["function"] = this->Location.Function;
    }

    if (!src.empty()) {
        value["source"] = src;
    }

    nlohmann::json fields;
    for (const auto& [name, field]: this->Fields) {
        nlohmann::json fieldValue;

        // clang-format off
        field.n_data.Match(
            [](std::monostate) -> void {},
            [&fieldValue](const bool& value) -> void { fieldValue = value; },
            [&fieldValue](const violet::UInt64& num) -> void { fieldValue = num; },
            [&fieldValue](const violet::Int64& num) -> void { fieldValue = num; },
            [&fieldValue](const violet::String& num) -> void { fieldValue = num; },
            [&fieldValue](const double& num) -> void { fieldValue = num; }
        );
        // clang-format on

        if (!fieldValue.is_null()) {
            fields[name] = fieldValue;
        }
    }

    if (!fields.is_null()) {
        value["attributes"] = fields;
    }

    return value;
}
