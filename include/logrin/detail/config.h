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

// Logrin only guarantees support for the following:
// * MSVC (Visual Studio 2019 or higher)
// * Clang 20
// * GCC 14

/**
 * @macro LOGRIN_MIN_CPP_VERSION
 *
 * This macro defines the minimum C++ standard that Violet can be built with.
 */
#define LOGRIN_MIN_CPP_VERSION 202002L

/**
 * @macro LOGRIN_MIN_CLANG_MAJOR_SUPPORTED
 *
 * This macro defines the minimum Clang version that Violet supports
 */
#define LOGRIN_MIN_CLANG_MAJOR_SUPPORTED 20

/**
 * @macro LOGRIN_MIN_GCC_MAJOR_SUPPORTED
 *
 * This macro defines the minimum GCC version that Violet supports
 */
#define LOGRIN_MIN_GCC_MAJOR_SUPPORTED 14

/**
 * @macro LOGRIN_MIN_MSVC_VERSION
 *
 * This macro defines the minimum Visual Studio (MSVC) version that Violet supports
 */
#define LOGRIN_MIN_MSVC_VERSION 1920

#if defined(__clang__) && __clang_major__ < LOGRIN_MIN_CLANG_MAJOR_SUPPORTED
#error "Logrin aims to only support Clang 20 or higher"
#endif

#if (defined(__GNUC__) && !defined(__clang__)) && __GNUC__ < LOGRIN_MIN_GCC_MAJOR_SUPPORTED
#error "Logrin aims to only support GCC 14 or higher"
#endif

#if (defined(_MSC_VER) && !defined(__clang__)) && _MSC_VER < LOGRIN_MIN_MSVC_VERSION
#error "Logrin aims to only support Visual Studio 2019 or higher"
#endif

#ifdef _MSVC_LANG
#if _MSVC_LANG < LOGRIN_MIN_CPP_VERSION
#error "Logrin aims to only support C++ 20 or higher"
#endif
#elif defined(__cplusplus) && __cplusplus < LOGRIN_MIN_CPP_VERSION
#error "Logrin aims to only support C++ 20 or higher"
#endif

// Use the new conforming preprocessor on MSVC
#if defined(_MSC_VER) && !defined(__clang__) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#error "Logrin requires the use of the new conforming preprocessor enabled by the `/Zc:preprocessor` flag"
#endif

#if !defined(VIOLET_HAS_ATTRIBUTE) && defined(__has_attribute)
#define LOGRIN_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define LOGRIN_HAS_ATTRIBUTE(x) 0
#endif

#ifndef LOGRIN_API
#ifdef LOGRIN_STATIC
#define LOGRIN_API
#define LOGRIN_LOCAL
#elif defined(LOGRIN_WINDOWS) || defined(__CYGWIN__)
#ifdef LOGRIN_BUILDING
#define LOGRIN_API __declspec(dllexport)
#else
#define LOGRIN_API __declspec(dllimport)
#endif // !defined(LOGRIN_BUILDING)

#define LOGRIN_LOCAL
#elif LOGRIN_HAS_ATTRIBUTE(visibility)
#define LOGRIN_API __attribute__((visibility("default")))
#define LOGRIN_LOCAL __attribute__((visibility("hidden")))
#else
#define LOGRIN_API
#define LOGRIN_LOCAL
#endif // !defined(LOGRIN_STATIC)
#endif // !defined(LOGRIN_API)

#ifdef LOGRIN_STATIC
#define LOGRIN_EXTERN_TEMPLATE_DECL
#define LOGRIN_EXTERN_TEMPLATE_DEF
#elif defined(LOGRIN_WINDOWS) || defined(__CYGWIN__)
#define LOGRIN_EXTERN_TEMPLATE_DECL __declspec(dllimport)
#define LOGRIN_EXTERN_TEMPLATE_DEF __declspec(dllexport)
#elif LOGRIN_HAS_ATTRIBUTE(visibility)
#define LOGRIN_EXTERN_TEMPLATE_DECL __attribute__((visibility("default")))
#define LOGRIN_EXTERN_TEMPLATE_DEF __attribute__((visibility("default")))
#else
#define LOGRIN_EXTERN_TEMPLATE_DECL
#define LOGRIN_EXTERN_TEMPLATE_DEF
#endif // defined(LOGRIN_STATIC)

#if LOGRIN_HAS_ATTRIBUTE(deprecated)
#define LOGRIN_DEPRECATED(since, message) [[deprecated("since " since ": " message)]]
#else
#define LOGRIN_DEPRECATED(since, message)
#endif

#define __logrin_concat_inner__(x, y) x##y
#define LOGRIN_CONCAT(x, y) __logrin_concat_inner__(x, y)

#define LOGRIN_FEATURE(feature) LOGRIN_CONCAT(LOGRIN_FEATURE_, feature)
