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
#include <logrin/LogFactory.h>
#include <logrin/LogRecord.h>
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

TEST(LogFactory, ItWorks)
{
    auto* sink = new MockSink();
    auto* async = new AsyncMockSink();

    LogFactory::Init(LogLevel::Trace, { sink }, { async });

    auto log = LogFactory::Get("logger");
    log.Fatal("systems crashed: doin ur mom");

    EXPECT_EQ(sink->Emitted, 1);
    EXPECT_EQ(async->Enqueued, 1);

    LogFactory::Shutdown();
}
