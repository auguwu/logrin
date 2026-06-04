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

#include <gtest/gtest.h>
#include <logrin/Sinks/OpenTelemetry.h>
#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/logger.h>

namespace common = opentelemetry::common;
namespace otel = opentelemetry::logs;
namespace nostd = opentelemetry::nostd;
namespace trace = opentelemetry::trace;
namespace logs = opentelemetry::logs;

// NOLINTBEGIN(google-build-using-namespace,cppcoreguidelines-pro-type-static-cast-downcast)
using namespace violet;
using namespace logrin;
using namespace logrin::sinks;

namespace {

struct CapturedRecord final {
    otel::Severity Severity = otel::Severity::kInvalid;
    String Body;
    Vec<Pair<String, String>> Attributes;
};

struct MockLogRecord final: public otel::LogRecord {
    CapturedRecord Record;

    void SetSeverity(logs::Severity severity) noexcept override
    {
        Record.Severity = severity;
    }

    void SetBody(const common::AttributeValue& body) noexcept override
    {
        if (const auto* sv = nostd::get_if<nostd::string_view>(&body)) {
            Record.Body.assign(sv->data(), sv->size());
        }
    }

    void SetAttribute(nostd::string_view key, const common::AttributeValue& value) noexcept override
    {
        String val;
        if (const auto* sv = nostd::get_if<nostd::string_view>(&value)) {
            val.assign(sv->data(), sv->size());
        } else if (const auto* int_ = nostd::get_if<int64_t>(&value)) {
            val = violet::ToString(*int_);
        }

        Record.Attributes.emplace_back(String(key), VIOLET_MOVE(val));
    }

    void SetObservedTimestamp(common::SystemTimestamp timestamp) noexcept override
    {
        Record.Attributes.emplace_back("@timestamp", violet::ToString(timestamp.time_since_epoch().count()));
    }

    void SetTimestamp(common::SystemTimestamp timestamp) noexcept override { }
    void SetEventId(int64_t id, nostd::string_view name) noexcept override { }
    void SetTraceId(const trace::TraceId& trace_id) noexcept override { }
    void SetSpanId(const trace::SpanId& span_id) noexcept override { }
    void SetTraceFlags(const trace::TraceFlags& trace_flags) noexcept override { }
};

struct MockLogger final: public otel::Logger {
    struct experimental::Mutex Mutex;
    Vec<CapturedRecord> Records;

    auto GetName() noexcept -> const nostd::string_view override
    {
        return "mock";
    }

    auto CreateLogRecord() noexcept -> nostd::unique_ptr<otel::LogRecord> override
    {
        return nostd::unique_ptr<otel::LogRecord>(new MockLogRecord());
    }

    void EmitLogRecord(nostd::unique_ptr<otel::LogRecord>&& rec) noexcept override
    {
        experimental::MutexLock lock(this->Mutex);
        auto record = VIOLET_MOVE(rec);
        auto* mock = static_cast<MockLogRecord*>(record.get());
        Records.push_back(mock->Record);
    }
};

} // namespace

TEST(OTelSink, ForwardsRecords)
{
    auto logger = nostd::shared_ptr<otel::Logger>(new MockLogger());
    auto* mock = static_cast<MockLogger*>(logger.get());
    {
        OpenTelemetry sink(logger);
        sink.Enqueue(LogRecord::Now(LogLevel::Warning, "w"));
        sink.Flush();
    }

    EXPECT_EQ(mock->Records.size(), 1U);
    EXPECT_EQ(mock->Records[0].Severity, logs::Severity::kWarn);
    EXPECT_EQ(mock->Records[0].Body, "w");
    EXPECT_FALSE(mock->Records[0].Attributes.empty());
}

TEST(OTelSink, ForwardsRecordsWithAttributes)
{
    auto logger = nostd::shared_ptr<otel::Logger>(new MockLogger());
    auto* mock = static_cast<MockLogger*>(logger.get());
    {
        OpenTelemetry sink(logger);
        sink.Enqueue(LogRecord::Now(LogLevel::Warning, "w").With("hello", "world").With("uwu", 69));
        sink.Flush();
    }

    EXPECT_EQ(mock->Records.size(), 1U);
    EXPECT_EQ(mock->Records[0].Severity, logs::Severity::kWarn);
    EXPECT_EQ(mock->Records[0].Body, "w");
    EXPECT_FALSE(mock->Records[0].Attributes.empty());
    EXPECT_EQ(mock->Records[0].Attributes[1], Pair("uwu", "69"));
    EXPECT_EQ(mock->Records[0].Attributes[2], Pair("hello", "world"));
}

// NOLINTEND(google-build-using-namespace,cppcoreguidelines-pro-type-static-cast-downcast)
