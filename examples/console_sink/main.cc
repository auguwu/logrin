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

#include <logrin/Formatter/Json.h>
#include <logrin/Formatter/Pattern.h>
#include <logrin/LogFactory.h>
#include <logrin/Sinks/Console.h>

// NOLINTBEGIN(google-build-using-namespace)

using namespace logrin;
using namespace logrin::sinks;
using namespace logrin::formatter;

using namespace violet;

auto main() -> int
{
    LogFactory::Init(LogLevel::Info,
        { std::make_shared<Console>(Json()), std::make_shared<Console>(Console::Stream::Stderr, Pattern::Azalia()),
            std::make_shared<Console>(
                Console::Stream::Stderr, Pattern::Parse("%levelColor[%L{upper:^10}]%style:end :: %m\t%A").Unwrap()) });

    auto logger = LogFactory::Get("console_sink");
    logger.Info("Hello, world!").With("disk", "/dev/sda1");

    auto _67 = logger.Fatal("im not funny");
    LogFactory::Shutdown();
}

// NOLINTEND(google-build-using-namespace)
