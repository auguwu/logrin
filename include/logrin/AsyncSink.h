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

/// An asynchronous version of [`logrin::Sink`].
///
/// AsyncSink represents a output stream that processes [log records][logrin::LogRecord] asynchronously,
/// typically by enqueueing them into a background worker, thread pool, or event loop.
///
/// Unlike [`logrin::Sink`], implementations of `AsyncShould` should ***never block*** when
/// calling [`logrin::AsyncSink::Enqueue`]. Instead, records should transfer ownership or copy
/// of the log record into an internal queue and return immediately.
///
/// ## Safety
/// Implementations must ensure that both `Enqueue` and `Flush` are safe to call concurrently
/// from multiple threads.
///
/// Destructors must not return until all owned resources (threads, file handles, etc) have
/// been released.
struct AsyncSink {
    /// Destroys the sink and releases all owned resources.
    ///
    /// Implementations must ensure that all background workers have terminated
    /// and no queued records are still being processed.
    virtual ~AsyncSink() = default;

    /// Enqueues a log record for asynchronous processing.
    ///
    /// This function **must not block** on I/o or long-running work. The record
    /// should be copied or moved into an internal queue for later processing.
    ///
    /// @param record the log record to process
    virtual void Enqueue(const LogRecord& record) = 0;

    /// Flushes all queued log records.
    ///
    /// Implementations should block until all previously enqueued records
    /// have been fully processed and any buffered output has been written.
    virtual void Flush() noexcept {}
};

} // namespace logrin
