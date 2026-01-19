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

#include <violet/Language/Macros.h>

namespace {

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
#if VIOLET_HAS_ATTRIBUTE(visibility)
#define LOGRIN_HIDE __attribute__((visibility("hidden")))
#endif

VIOLET_DIAGNOSTIC_PUSH
VIOLET_DIAGNOSTIC_IGNORE("-Wunused-function")

#elif defined(VIOLET_MSVC)
#define LOGRIN_HIDE
#endif

LOGRIN_HIDE void __dummy()
{
    // this will be hidden by the linker, this is force Bazel
    // to build `//:logrin` as both a dynamic and static library
}

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
VIOLET_DIAGNOSTIC_POP
#endif

} // namespace
