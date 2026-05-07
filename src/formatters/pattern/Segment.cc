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

#include <logrin/Formatter/Pattern/Segment.h>

using logrin::formatter::pattern::Alignment;
using logrin::formatter::pattern::Directive;
using logrin::formatter::pattern::Error;
using logrin::formatter::pattern::SegmentCapability;
using logrin::formatter::pattern::SegmentKind;

using violet::Result;
using violet::Str;
using violet::UInt16;
using violet::UInt32;

#define error(message) ::violet::Err<::logrin::formatter::pattern::Error>(::violet::String(message))
#define errfmt(fmt, ...)                                                                                               \
    ::violet::Err<::logrin::formatter::pattern::Error>(::std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#if VIOLET_REQUIRE_STL(202302L)
#define contains(input, ch) (input.contains(ch))
#else
#define contains(input, ch) (input.find(ch) != Str::npos)
#endif

namespace {

constexpr auto isAlignmentCharacter(char ch) noexcept -> bool
{
    return ch == '<' || ch == '>' || ch == '^';
}

constexpr auto isDigit(char ch) noexcept -> bool
{
    return ch >= '0' && ch <= '9';
}

constexpr auto getAlignmentFromCharacter(char ch) noexcept -> Alignment
{
    switch (ch) {
    case '<':
        return Alignment::Left;
    case '>':
        return Alignment::Right;
    case '^':
        return Alignment::Center;
    default:
        return Alignment::None;
    }
}

auto parsePadding(Str input, Directive& directive) -> violet::Result<void, Error>
{
    if (input.empty()) {
        return { };
    }

    violet::UInt pos = 0;
    if (input.size() >= 2 && isAlignmentCharacter(input[1])) {
        directive.Fill = input[0];
        directive.Alignment = getAlignmentFromCharacter(input[1]);
        pos = 2;
    } else if (isAlignmentCharacter(input[0])) {
        directive.Alignment = getAlignmentFromCharacter(input[0]);
        pos = 1;
    } else if (input[0] == '0' && input.size() > 1) {
        directive.Fill = '0';
        directive.Alignment = Alignment::Right;
        pos = 1;
    }

    UInt16 width = 0;
    bool hasDigits = false;
    while (pos < input.size()) {
        char ch = input[pos];
        if (!isDigit(ch)) {
            return errfmt("invalid character [{}] in width specifier", ch);
        }

        width = (width * 10) + static_cast<violet::UInt16>(ch - '0');
        hasDigits = true;
        ++pos;
    }

    if (!hasDigits && directive.Alignment != Alignment::None) {
        return error("padding alignment requires a width");
    }

    if (hasDigits && directive.Alignment == Alignment::None) {
        directive.Alignment = Alignment::Right;
    }

    directive.Width = width;
    return { };
}

} // namespace

auto Directive::Parse(Str input, UInt32 offset, SegmentKind kind) -> Result<Directive, Error>
{
    auto capability = GetSegmentCapability(kind);
    if (capability == SegmentCapability::None) {
        return errfmt("directive with specification [{}] does not accept a specification", input);
    }

    Directive directive;
    if (input.empty()) {
        return directive;
    }

    switch (capability) {
    case SegmentCapability::SpecOnly: {
        directive.SpecOffset = static_cast<UInt16>(offset);
        directive.SpecLength = static_cast<UInt16>(input.size());

        return directive;
    }

    case SegmentCapability::PaddingOnly: {
        if (contains(input, ':')) {
            return error("directive does not accept a format specification; use padding syntax (e.g., `%n{<20}`)");
        }

        VIOLET_TRY_VOID(parsePadding(input, directive));
        return directive;
    }

    case SegmentCapability::SpecAndPadding: {
        auto colon = input.rfind(':');
        if (colon == Str::npos) {
            // No `:` was specified, we'll disambiguate between spec and padding. If it looks
            // like padding syntax, parse as padding. Otherwise, parse as specification.
            if (!input.empty()
                && (
                    // `^`, `>`, `<`
                    isAlignmentCharacter(input[0]) ||

                    // 0..9
                    isDigit(input[0]) ||

                    // [alignment][digit]
                    (input.size() >= 2 && isAlignmentCharacter(input[1])))) {
                VIOLET_TRY_VOID(parsePadding(input, directive));
                return directive;
            }

            directive.SpecOffset = static_cast<UInt16>(offset);
            directive.SpecLength = static_cast<UInt16>(input.size());

            return directive;
        }

        auto spec = input.substr(0, colon);
        auto padding = input.substr(colon + 1);
        if (!spec.empty()) {
            directive.SpecOffset = static_cast<UInt16>(offset);
            directive.SpecLength = static_cast<UInt16>(spec.size());
        }

        if (!padding.empty()) {
            VIOLET_TRY_VOID(parsePadding(padding, directive));
        }

        return directive;
    }

    default:
        VIOLET_UNREACHABLE();
    }
}
