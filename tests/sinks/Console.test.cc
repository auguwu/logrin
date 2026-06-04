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
#include <logrin/Sinks/Console.h>
#include <violet/Filesystem/Temporary.h>

#if VIOLET_PLATFORM(UNIX)
#include <unistd.h>

#define STDOUT_HANDLE STDOUT_FILENO
#define STDERR_HANDLE STDERR_FILENO
#elif VIOLET_PLATFORM(WINDOWS)
#include <windows.h>

#define STDOUT_HANDLE STD_OUTPUT_HANDLE
#define STDERR_HANDLE STD_ERROR_HANDLE
#else
#error "add support for your platform for `{STDOUT|STDERR}_HANDLE`"
#endif

// NOLINTBEGIN(google-build-using-namespace)
using namespace logrin::sinks;
using namespace logrin;
using namespace violet;

namespace fs = violet::filesystem;
namespace io = violet::io;

namespace {
struct CaptureFileDescriptorStream final {
    VIOLET_DISALLOW_COPY_AND_MOVE(CaptureFileDescriptorStream);

    fs::TempFile Temporary;
    io::FileDescriptor::value_type FileDescriptor;
    io::FileDescriptor::value_type SavedFD;

    VIOLET_EXPLICIT CaptureFileDescriptorStream(io::FileDescriptor::value_type fd)
        : Temporary(fs::TempBuilder{ }.WithPrefix("logrin-").MkFile().Unwrap())
        , FileDescriptor(fd)
        , SavedFD(::dup(fd))
    {
        ::dup2(Temporary.File().Descriptor(), fd); // point `fd` to the temp file
    }

    ~CaptureFileDescriptorStream()
    {
        // restore
        ::dup2(SavedFD, FileDescriptor);
        ::close(SavedFD);
    }

    auto Get(SourceLocation loc = std::source_location::current()) const -> io::Result<String>
    {
        testing::ScopedTrace trace(loc.File.data(), static_cast<Int32>(loc.Line), "reading temporary file");

        (void)Temporary.File().Flush();

        auto path = Temporary.Path();
        auto reader = fs::File::Open(*path, fs::OpenOptions{ }.Read()).Unwrap();
        return io::ReadToString(reader);
    }
};

struct SpyFormatter final: public Formatter {
    SharedPtr<UInt32> Calls = std::make_shared<UInt32>(0);

    [[nodiscard]] auto Format(const LogRecord&) const -> String override
    {
        (*this->Calls)++;
        return "x";
    }
};
} // namespace

TEST(ConsoleSink, WritesFormattedLineToStdout)
{
    CaptureFileDescriptorStream captured(STDOUT_HANDLE);
    Console console(Console::Stream::Stdout, SpyFormatter());

    console.Emit(LogRecord::Now(LogLevel::Info, "message"));
    console.Flush();

    auto str = captured.Get();
    ASSERT_TRUE(str) << "failed to read stdout: " << str.Error();
    EXPECT_EQ(*str, "x\n");
}

// NOLINTEND(google-build-using-namespace)
