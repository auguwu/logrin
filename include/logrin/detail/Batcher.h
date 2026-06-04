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

#include <violet/Experimental/Mutex.h>

#include <functional>
#include <queue>
#include <thread>

namespace logrin::detail {

/// A single-consumer background worker that drains queued items of type `T` and
/// hands them to a consumer callback on a dedicated thread.
template<typename T>
struct Batcher final {
    /// a **consumer** receives a non-empty batch drained from the queue
    using consumer = std::function<void(violet::Vec<T>&&)>;

    struct Options {
        /// The maximum amount of items handed to the consumer per call. `0` means
        /// drain everything.
        violet::UInt MaxBatchSize = 0;
    };

    VIOLET_DISALLOW_COPY_AND_MOVE(Batcher);
    VIOLET_DISALLOW_CONSTRUCTOR(Batcher);
    VIOLET_IMPLICIT Batcher(consumer fn, Options opts = { }) noexcept
        : n_running(true)
        , n_options(opts)
        , n_consumer(VIOLET_MOVE(fn))
    {
        this->n_worker = std::thread([this] -> void { this->workerLoop(); });
    }

    ~Batcher()
    {
        {
            violet::experimental::MutexLock lock(this->n_mux);
            this->n_running = false;
        }

        this->n_cv.SignalAll();
        if (this->n_worker.joinable()) {
            this->n_worker.join();
        }
    }

    void Push(T item)
    {
        {
            violet::experimental::MutexLock lock(this->n_mux);
            this->n_queue.push(VIOLET_MOVE(item));
        }

        this->n_cv.Signal();
    }

    void Flush() noexcept
    {
        violet::experimental::MutexLock lock(this->n_mux);
        while (!this->n_queue.empty() || this->n_processing) {
            this->n_cv.Wait(&this->n_mux);
        }
    }

private:
    std::thread n_worker;
    violet::experimental::Condvar n_cv;
    violet::experimental::Mutex n_mux;
    std::queue<T> n_queue;
    std::atomic<bool> n_running;
    bool n_processing = false;
    Options n_options;
    consumer n_consumer;

    void workerLoop()
    {
        this->n_mux.Lock();
        while (true) {
            while (this->n_queue.empty() && this->n_running) {
                this->n_cv.Wait(&this->n_mux);
            }

            if (this->n_queue.empty() && !this->n_running) {
                break;
            }

            violet::Vec<T> batch;
            const violet::UInt batchLimit = this->n_options.MaxBatchSize;
            while (!this->n_queue.empty() && (batchLimit == 0 || batch.size() < batchLimit)) {
                batch.push_back(VIOLET_MOVE(this->n_queue.front()));
                this->n_queue.pop();
            }

            this->n_processing = true;
            this->n_mux.Unlock();
            std::invoke(this->n_consumer, VIOLET_MOVE(batch));

            this->n_mux.Lock();
            this->n_processing = false;
            this->n_cv.SignalAll();
        }

        this->n_mux.Unlock();
    }
};

} // namespace logrin::detail
