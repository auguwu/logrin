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

#include "bits/Macros.h"

#include <violet/Violet.h>

namespace logrin {

struct Sink;
struct AsyncSink;
struct LogRecord;

struct LOGRIN_API Logger final {
    VIOLET_DISALLOW_CONSTRUCTOR(Logger);
    VIOLET_IMPLICIT_COPY_AND_MOVE(Logger);

    VIOLET_IMPLICIT Logger(violet::Str name) noexcept;
    VIOLET_IMPLICIT Logger(violet::Str name, std::initializer_list<Sink*> sinks,
        std::initializer_list<AsyncSink*> asyncSinks = {}) noexcept;

    ~Logger();

    template<typename SinkT>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(SinkT* sink) noexcept -> Logger&
    {
        this->n_sinks.emplace_back(sink);
        return *this;
    }

    template<typename SinkT, typename... Args>
        requires(!std::is_abstract_v<SinkT> && std::is_base_of_v<Sink, SinkT>)
    auto AddSink(Args&&... args) noexcept(std::is_constructible_v<SinkT, Args...>) -> Logger&
    {
        this->n_sinks.emplace_back(std::make_shared<SinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    template<typename AsyncSinkT, typename... Args>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(Args&&... args) noexcept(std::is_constructible_v<AsyncSinkT, Args...>) -> Logger&
    {
        this->n_asyncSinks.emplace_back(std::make_shared<AsyncSinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    template<typename AsyncSinkT>
        requires(!std::is_abstract_v<AsyncSinkT> && std::is_base_of_v<AsyncSink, AsyncSinkT>)
    auto AddAsyncSink(AsyncSinkT* sink) noexcept -> Logger&
    {
        this->n_asyncSinks.emplace_back(sink);
        return *this;
    }

    auto WithName(violet::Str name) noexcept -> Logger&;
    [[nodiscard]] auto Sinks() const noexcept -> violet::Span<const violet::SharedPtr<Sink>>;
    [[nodiscard]] auto AsyncSinks() const noexcept -> violet::Span<const violet::SharedPtr<AsyncSink>>;
    [[nodiscard]] auto Name() const noexcept -> violet::Str;

    void Log(const LogRecord& record);

private:
    friend struct LogFactory;

    violet::String n_name;
    violet::Vec<violet::SharedPtr<Sink>> n_sinks;
    violet::Vec<violet::SharedPtr<AsyncSink>> n_asyncSinks;

    void flush();
};

} // namespace logrin
