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

#pragma once

#include <logrin/Sink.h>
#include <logrin/Sinks/Console/Formatter.h>
#include <violet/Violet.h>

namespace logrin::sinks {

struct VIOLET_API Console final: public Sink {
    VIOLET_IMPLICIT Console() noexcept = default;

    template<typename T, typename... Args>
        requires(std::is_base_of_v<console::Formatter, T> && std::is_constructible_v<T, Args...>)
    auto WithFormatter(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        this->n_formatter = std::make_shared<T>(VIOLET_FWD(Args, args)...);
        return *this;
    }

    /// @see logrin::Sink::Emit(const logrin::LogRecord&)
    void Emit(const LogRecord& record) override;

    /// @see logrin::Sink::Flush()
    void Flush() noexcept override;

private:
    violet::SharedPtr<console::Formatter> n_formatter;
};

} // namespace logrin::sinks
