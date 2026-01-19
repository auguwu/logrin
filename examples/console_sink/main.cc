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

#include <logrin/LogFactory.h>
#include <logrin/Sinks/Console.h>
#include <logrin/Sinks/Console/Formatter/Azalia.h>
#include <logrin/Sinks/Console/Formatter/Json.h>

using namespace logrin; // NOLINT(google-build-using-namespace)
using namespace logrin::sinks::console::formatters; // NOLINT(google-build-using-namespace)

auto main() -> int
{
    auto* console = new sinks::Console();
    *console = console->WithFormatter<Azalia>();

    auto* console2 = new sinks::Console();
    *console2 = console2->WithFormatter<Json>();

    LogFactory::Init(LogLevel::Info, { console, console2 });

    auto log = LogFactory::Get("console_sink");
    log.Info("Hello, world! (this should be emitted right away)").With("disk", "/dev/sda1");

    auto number_67 = log.Error("this should be emitted last? 67!! haha!! funn number!!!! im not funny...");

    LogFactory::Shutdown();
}
