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

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#ifndef LOGRIN_API
#ifdef LOGRIN_STATIC
#define LOGRIN_API
#define LOGRIN_LOCAL
#elif defined(VIOLET_WINDOWS) || defined(__CYGWIN__)
#ifdef LOGRIN_BUILDING
#define LOGRIN_API __declspec(dllexport)
#else
#define LOGRIN_API __declspec(dllimport)
#endif // !defined(LOGRIN_BUILDING)

#define VIOLET_LOCAL
#elif VIOLET_HAS_ATTRIBUTE(visibility)
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
#elif defined(VIOLET_WINDOWS) || defined(__CYGWIN__)
#define VIOLET_EXTERN_TEMPLATE_DECL __declspec(dllimport)
#define VIOLET_EXTERN_TEMPLATE_DEF __declspec(dllexport)
#elif VIOLET_HAS_ATTRIBUTE(visibility)
#define VIOLET_EXTERN_TEMPLATE_DECL __attribute__((visibility("default")))
#define VIOLET_EXTERN_TEMPLATE_DEF __attribute__((visibility("default")))
#else
#define LOGRIN_EXTERN_TEMPLATE_DECL
#define LOGRIN_EXTERN_TEMPLATE_DEF
#endif // defined(LOGRIN_STATIC)
