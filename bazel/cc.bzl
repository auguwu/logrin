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

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(":version.bzl", "DEVBUILD", "encode_as_int")

SANITIZER_OPTS = select({
    "//bazel/flags:asan_enabled": ["-fsanitize=address"],
    "//conditions:default": [],
}) + select({
    "//bazel/flags:msan_enabled": [
        "-fsanitize=memory",
        "-fsanitize-memory-track-origins",
        "-fsanitize-memory-use-after-dtor",
    ],
    "//conditions:default": [],
}) + select({
    "//bazel/flags:tsan_enabled": ["-fsanitize=thread"],
    "//conditions:default": [],
}) + select({
    "//bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
    "//conditions:default": [],
})

UBSAN_OPTIONS = [
    "halt_on_error=1",
    "print_summary=1",
    "print_stacktrace=1",
]

TSAN_OPTIONS = [
    "halt_on_error=1",
    "print_summary=1",
    "second_deadlock_state=1",
    "report_atomic_races=0",
]

MSAN_OPTIONS = [
    "poison_in_dtor=1",
]

LSAN_OPTIONS = [
    "report_objects=1",
    "print_summary=1",
]

ASAN_OPTIONS = [
    "detect_leaks=1",
    "color=always",
    "print_summary=1",
]

SANITIZER_ENV = select({
    "//bazel/flags:ubsan_enabled": {
        "UBSAN_OPTIONS": ":".join(UBSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//bazel/flags:asan_enabled": {
        "ASAN_OPTIONS": ":".join(ASAN_OPTIONS),
        "LSAN_OPTIONS": ":".join(LSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//bazel/flags:tsan_enabled": {
        "TSAN_OPTIONS": ":".join(TSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//bazel/flags:tsan_enabled": {
        "MSAN_OPTIONS": ":".join(MSAN_OPTIONS),
    },
    "//conditions:default": {},
})

COMPILER_COPTS = select({
    "@rules_cc//cc/compiler:clang": [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
        "-DNOMINMAX",
    ],
    "@rules_cc//cc/compiler:clang-cl": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:gcc": [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
        "-DNOMINMAX",
    ],
    "@rules_cc//cc/compiler:msvc-cl": [
        "/DNOMINMAX",
        "/DWIN32_LEAN_AND_MEAN",
        "/Zc:dllexportInlines-",
    ],
    "//conditions:default": [],
})

def logrin_cc_library(
        name,
        deps = [],
        copts = [],
        linkopts = [],
        local_defines = [],
        defines = [],
        includes = [],
        **kwargs):
    """A `cc_library` wrapper for Logrin's internal targets.

    This will:
        - Add logrin's shared dependencies (`//:config`, `//:include_hack`)
        - Set `LOGRIN_BUILDING` for producer-side macro dispatch
        - Propagate `LOGRIN_STATIC` to consumers when static mode is active (`--@logrin//bazel/flags:static=True`)
        - Applies sanitizer and compiler-wide options
        - Forbid callers from passing in `"include"` in `includes`.
    """

    # Remove `includes` from any `cc_library` definition.
    if "include" in includes:
        fail("do not append `include` in `\"includes\"`, it adds it automatically")

    return cc_library(
        name = name,
        copts = copts + SANITIZER_OPTS + COMPILER_COPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        includes = ["include"] + includes,
        local_defines = local_defines + ["LOGRIN_BUILDING"],
        defines = [
            "BAZEL",
            "LOGRIN_VERSION=%d" % encode_as_int(),
            "LOGRIN_DEVBUILD=%d" % (1 if DEVBUILD else 0),
        ] + select({
            "//bazel/flags:static_enabled": ["LOGRIN_STATIC"],
            "//conditions:default": [],
        }) + defines,
        deps = ["//:include_hack", "//:config"] + deps,
        **kwargs
    )

def logrin_cc_test(
        name,
        deps = [],
        size = "small",
        copts = [],
        linkopts = [],
        env = {},
        with_gtest_main = True,
        **kwargs):
    # buildifier: disable=list-append
    deps += ["@googletest//:gtest"]
    if with_gtest_main:
        # buildifier: disable=list-append
        deps += ["@googletest//:gtest_main"]

    if "visibility" in kwargs:
        fail("remove `visibility` as all tests will be marked private")

    return cc_test(
        name = name,
        deps = deps,
        copts = copts + SANITIZER_OPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        env = env | SANITIZER_ENV,
        visibility = ["//visibility:private"],
        size = size,
        **kwargs
    )
