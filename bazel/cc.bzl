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

load("@rules_cc//cc:defs.bzl", cc_library_ = "cc_library", cc_test_ = "cc_test")

SANITIZER_OPTS = select({
    "//bazel/flags:asan_enabled": ["-fsanitize=address"],
    "//bazel/flags:msan_enabled": ["-fsanitize=memory"],
    "//bazel/flags:tsan_enabled": ["-fsanitize=thread"],
    "//bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
    "//conditions:default": [],
})

MSAN_OPTS = []
UBSAN_OPTS = ["print_summary=1", "print_stacktrace=1"]
ASAN_OPTS = ["print_summary=1"]
TSAN_OPTS = ["print_summary=1"]

SANITIZER_ENV = select({
    "//bazel/flags:ubsan_enabled": {"UBSAN_OPTIONS": ":".join(UBSAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//bazel/flags:asan_enabled": {"ASAN_OPTIONS": ":".join(ASAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//bazel/flags:tsan_enabled": {"TSAN_OPTIONS": ":".join(TSAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//bazel/flags:tsan_enabled": {"MSAN_OPTIONS": ":".join(MSAN_OPTS)},
    "//conditions:default": {},
})

COMPILER_COPTS = select({
    "@rules_cc//cc/compiler:clang": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:clang-cl": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:gcc": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:msvc-cl": ["/DNOMINMAX", "/DWIN32_LEAN_AND_MEAN"],
    "//conditions:default": [],
})

def cc_library(name, **kwargs):
    copts = kwargs.pop("copts", [])
    linkopts = kwargs.pop("linkopts", [])

    deps = kwargs.pop("deps", [])

    # buildifier: disable=list-append
    deps += ["//:include_hack"]

    # Remove `includes` from any `cc_library` definition.
    # buildifier: disable=unused-variable
    _ = kwargs.pop("includes", [])

    return cc_library_(
        name = name,
        copts = copts + SANITIZER_OPTS + COMPILER_COPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        includes = ["include"],
        defines = ["BAZEL"],
        deps = deps,
        **kwargs
    )

def cc_test(name, with_gtest_main = True, **kwargs):
    deps = kwargs.pop("deps", [])
    deps.append("@googletest//:gtest")

    if with_gtest_main:
        deps.append("@googletest//:gtest_main")

    # remove `visibility` in `cc_test` and make them private
    kwargs.pop("visibility", [])

    # set `size` if it is not defined
    size = kwargs.pop("size", "small")

    copts = kwargs.pop("copts", [])
    linkopts = kwargs.pop("linkopts", [])
    env = kwargs.pop("env", {})

    return cc_test_(
        name = name,
        deps = deps,
        copts = copts + SANITIZER_OPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        env = env | SANITIZER_ENV,
        visibility = ["//visibility:private"],
        size = size,
        **kwargs
    )
