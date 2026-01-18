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
#include <logrin/AsyncSink.h>
#include <logrin/LogRecord.h>
#include <logrin/Logger.h>
#include <logrin/Sink.h>

using namespace logrin; // NOLINT(google-build-using-namespace)

struct MockSink final: public logrin::Sink {
    violet::UInt32 Emitted = 0;

    void Emit(const LogRecord&) override
    {
        this->Emitted++;
    }
};

struct AsyncMockSink final: public logrin::AsyncSink {
    violet::UInt32 Enqueued = 0;

    void Enqueue(const LogRecord&) override
    {
        this->Enqueued++;
    }
};

TEST(Loggers, DefaultConstructor)
{
    Logger log("test");
    EXPECT_EQ(log.Name(), "test");
}

TEST(Loggers, AddSink)
{
    auto* sink = new MockSink();

    Logger log("test");
    log.AddSink(sink);

    LogRecord record = LogRecord::Now(LogLevel::Fatal, "systems overloaded: ur mom gay").WithLogger(log.Name());
    log.Log(record);

    EXPECT_EQ(sink->Emitted, 1);
}

TEST(Loggers, AddAsyncSink)
{
    auto* sink = new AsyncMockSink();

    Logger log("test");
    log.AddAsyncSink(sink);

    LogRecord record = LogRecord::Now(LogLevel::Fatal, "systems overloaded: ur mom gay").WithLogger(log.Name());
    log.Log(record);

    EXPECT_EQ(sink->Enqueued, 1);
}

TEST(Loggers, EmitMultipleSinks)
{
    Logger log("multi");

    log.AddSink<MockSink>();
    log.AddSink<MockSink>();
    log.AddAsyncSink<AsyncMockSink>();
    log.AddAsyncSink<AsyncMockSink>();

    LogRecord record = LogRecord::Now(LogLevel::Fatal, "systems overloaded: ur mom gay").WithLogger(log.Name());
    log.Log(record);

    for (const auto& sink: log.Sinks()) {
        auto* mockSink = dynamic_cast<MockSink*>(sink.get());
        ASSERT_TRUE(mockSink != nullptr);
        EXPECT_EQ(mockSink->Emitted, 1);
    }

    for (const auto& sink: log.AsyncSinks()) {
        auto* mockSink = dynamic_cast<AsyncMockSink*>(sink.get());
        ASSERT_TRUE(mockSink != nullptr);
        EXPECT_EQ(mockSink->Enqueued, 1);
    }
}
