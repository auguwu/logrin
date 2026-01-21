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

#include <logrin/Sinks/Console.h>

#ifdef VIOLET_UNIX
#define STDOUT_HANDLE STDOUT_FILENO
#define STDERR_HANDLE STDERR_FILENO
#elif defined(VIOLET_WINDOWS)
#include <windows.h>

#define STDOUT_HANDLE STD_OUTPUT_HANDLE
#define STDERR_HANDLE STD_ERROR_HANDLE
#endif

using logrin::sinks::Console;
using violet::Int32;
using violet::UInt8;
using violet::Vec;

auto Console::WithStream(Console::Stream stream) noexcept -> Console&
{
    std::lock_guard lock(this->n_mux);
    if (this->n_descriptor.Valid()) {
        auto fd = this->n_descriptor.Get();
        if (fd == STDOUT_HANDLE && stream == Console::Stream::Stdout) {
            return *this;
        }

        if (fd == STDERR_HANDLE && stream == Console::Stream::Stderr) {
            return *this;
        }

        this->n_descriptor.Close();
        this->n_descriptor = {};
    }

    this->n_descriptor = stream == Console::Stream::Stdout ? STDOUT_HANDLE : STDERR_HANDLE;
    return *this;
}

void Console::Emit(const LogRecord& record)
{
    if (auto* fmt = this->n_formatter.get(); this->n_descriptor.Valid()) {
        auto str = fmt->Format(record);

        {
            std::lock_guard lock(this->n_mux);

            Vec<UInt8> data(str.begin(), str.end());
            auto res = this->n_descriptor.Write(data);

            // In rare case scenarios, `Write()` could fail for some odd reason. We could
            // just ignore it and be done for the day, but I, unfortunately, like to know
            // when shit hits the fan. So, this will write to `std::cerr`, and if that fails,
            // then it's not my problem anymore.
            if (res.Err()) {
                std::cerr << "[logrin@fatal:" << record.Location.file_name() << ":" << record.Location.line()
                          << record.Location.column()
                          << "]: received fatal error when writing to stream: " << VIOLET_MOVE(res.Error()).ToString()
                          << '\n';
            }
        }
    }
}

void Console::Flush() noexcept
{
    // Flush could throw an error from the OS if anything happens and I, unfortunately,
    // still want to be notified in my own programs.
    if (auto res = this->n_descriptor.Flush(); res.Err()) {
        auto err = VIOLET_MOVE(res.Error());

        // TODO(@auguwu/Noel): this is a bug with the `Noelware.Violet.IO` framework
        // with `io::FileDescriptor::Flush`.
        //
        // https://github.com/Noelware/violet/issues/19
        if (err.RawOSError().HasValueAnd([](Int32 err) -> bool { return err != EINVAL; })) {
            std::cerr << "[logrin@fatal]: received fatal error when flushing to stream: " << err.ToString() << '\n';
        }
    }
}
