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

#include <logrin/LogRecord.h>

namespace logrin {

/// A synchronous log output target.
///
/// [`logrin::Sink`] represents a destination for [log records][logrin::LogRecord] that
/// are emitted ***synchronously*** on the calling thread. Typical examples include console
/// output with [`std::cout`], files, in-memory collectors for testing and much more.
///
/// ## Threading
/// Implementations of [`logrin::Sink`] should usually be thread-safe unless explicitly documentated
/// else-where if whether or not this requirement must be met. Logrin may call [`Sink::Emit`] concurrently
/// from multiple threads.
struct VIOLET_API Sink {
    /// Destructor of a sink.
    ///
    /// Implementations should ensure that all resources are released and any
    /// buffered output is flushed.
    virtual ~Sink() = default;

    /// Emit a log record to this sink.
    ///
    /// This method is called synchronously as part of the logging call. Implementations
    /// should avoid long-running or blocking operations.
    ///
    /// @param record the log record to emit.
    virtual void Emit(const LogRecord& record) = 0;

    /// Flushes any buffered output to the destination.
    ///
    /// The default implementation does nothing. Sinks that buffer output
    /// such as file or network sinks, should override this method to ensure
    /// that all pending data is written.
    virtual void Flush() noexcept {}
};

} // namespace logrin
