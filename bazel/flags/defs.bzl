# 🐻‍❄️🏳️‍⚧️ Logrin: Modern, async-aware logging framework for C++20
# Copyright (c) 2026 Noel Towa <cutie@floofy.dev>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

BOOL_FLAGS = {
    "asan": {
        "default": False,
        "doc": "Enables the **Address** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "msan": {
        "default": False,
        "doc": """Enables the **Memory** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.

        When invoked on C++ targets, the C++ standard library implementation will require to be compiled with MemorySanitizer. This will always fail in libstdc++, but libc++
        has MSan support, but you will need to compile it yourself; default toolchains of libc++ don't compile with MSan by default.""",
    },
    "opentelemetry": {
        "default": False,
        "doc": "Enable the OpenTelemetry sink implementation",
    },
    "static": {
        "default": False,
        "doc": "Only should be enabled on static builds.",
    },
    "tsan": {
        "default": False,
        "doc": "Enables the **Thread** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "ubsan": {
        "default": False,
        "doc": "Enables the **Undefined Behaviour** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "win32_dllexport": {
        "default": False,
        "doc": "When set to **true**, this will use `__declspec(dllexport)` on MSVC toolchains instead of `__declspec(dllimport)`. This is a no-op on non-MSVC toolchains.",
    },
}

FLAGS = [
    "ubsan",
    "asan",
    "msan",
    "tsan",
]
