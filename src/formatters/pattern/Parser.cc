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

#include <logrin/Formatter/Pattern/Parser.h>

#include <violet/Strings.h>

#include <thread>

using logrin::LogLevel;
using logrin::formatter::pattern::Alignment;
using logrin::formatter::pattern::Directive;
using logrin::formatter::pattern::Error;
using logrin::formatter::pattern::ParseConfig;
using logrin::formatter::pattern::ParsedPattern;
using logrin::formatter::pattern::Segment;
using logrin::formatter::pattern::SegmentKind;

using violet::Array;
using violet::Optional;
using violet::Pair;
using violet::Result;
using violet::Str;
using violet::String;
using violet::UInt;
using violet::UInt16;
using violet::UInt32;
using violet::UInt8;
using violet::terminal::RGB;

#define error(message) ::violet::Err<::logrin::formatter::pattern::Error>(::violet::String(message))
#define errfmt(fmt, ...)                                                                                               \
    ::violet::Err<::logrin::formatter::pattern::Error>(::std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#if VIOLET_REQUIRE_STL(202302L)
#define strContains(input, ch) (input.contains(ch))
#else
#define strContains(input, ch) (input.find(ch) != Str::npos)
#endif

namespace {

struct NamedColor {
    Str Name;
    struct RGB RGB;
};

struct Keyword final {
    Str Name;
    SegmentKind Kind;
};

constexpr Array<Keyword, 9> kKeywords = {
    Keyword{ .Name = "strikethrough", .Kind = SegmentKind::Strikethrough },
    { .Name = "levelColor", .Kind = SegmentKind::LevelColorStart },
    { .Name = "style:end", .Kind = SegmentKind::StyleEnd },
    { .Name = "underline", .Kind = SegmentKind::Underline },
    { .Name = "italic", .Kind = SegmentKind::Italic },
    { .Name = "bold", .Kind = SegmentKind::Bold },
    { .Name = "dim", .Kind = SegmentKind::Dim },
    { .Name = "fg", .Kind = SegmentKind::ForegroundColorStart },
    { .Name = "bg", .Kind = SegmentKind::BackgroundColorStart },
};

auto levelToString(LogLevel level, Str spec) -> String
{
    constexpr auto toUpper
        = [](char ch) -> char { return static_cast<char>(std::toupper(static_cast<unsigned char>(ch))); };

    constexpr auto toLower
        = [](char ch) -> char { return static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); };

    auto levelStr = violet::ToString(level);
    if (spec == "upper") {
        String out(levelStr);
        std::ranges::transform(out, out.begin(), toUpper);

        return out;
    }

    if (spec == "lower") {
        String out(levelStr);
        std::ranges::transform(out, out.begin(), toLower);

        return out;
    }

    return levelStr;
}

auto isWordBoundary(Str rest, UInt end) -> bool
{
    if (end >= rest.size()) {
        return true;
    }

    char ch = rest[end];
    return ch == '{' || ch == '%' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '[' || ch == ']' || ch == '('
        || ch == ')' || ch == '}' || ch == ':';
}

auto tryMatchKeyword(Str rest) -> Pair<UInt32, SegmentKind>
{
    for (const auto& kw: kKeywords) {
        if (rest.starts_with(kw.Name) && isWordBoundary(rest, kw.Name.size())) {
            return { static_cast<UInt32>(kw.Name.size()), kw.Kind };
        }
    }

    return { 0, SegmentKind::Literal };
}

auto isStyleStart(SegmentKind kind) -> bool
{
    switch (kind) {
    case SegmentKind::ForegroundColorStart:
    case SegmentKind::BackgroundColorStart:
    case SegmentKind::LevelColorStart:
    case SegmentKind::Bold:
    case SegmentKind::Dim:
    case SegmentKind::Italic:
    case SegmentKind::Underline:
    case SegmentKind::Strikethrough:
        return true;

    default:
        return false;
    }
}

void applyPadding(Str content, const Directive& directive, String& out)
{
    if (!directive.HasPadding() || content.size() >= directive.Width) {
        out.append(content);
        return;
    }

    auto total = directive.Width - static_cast<UInt16>(content.size());
    switch (directive.Alignment) {
    case Alignment::Left:
        out.append(content);
        out.append(total, directive.Fill);
        break;

    case Alignment::Right:
        out.append(total, directive.Fill);
        out.append(content);
        break;

    case Alignment::Center: {
        auto left = total / 2;
        auto right = total - left;

        out.append(left, directive.Fill);
        out.append(content);
        out.append(right, directive.Fill);
    } break;

    default:
        return;
    }
}

} // namespace

auto logrin::formatter::pattern::Parse(violet::Str input, const ParseConfig& config)
    -> violet::Result<ParsedPattern, Error>
{
    ParsedPattern result;
    result.Pattern = input;

    UInt32 in = 0;
    UInt32 literalStart = 0;
    bool inLiteral = false;
    UInt8 depth = 0;
    auto len = static_cast<UInt32>(input.size());

    auto flushLiteral = [&literalStart, &depth, &result, &inLiteral](UInt32 end) -> Result<void, Error> {
        if (inLiteral && end > literalStart) {
            if (result.Count >= result.Segments.size()) {
                return error("pattern exceeds maximum segment count (64)");
            }

            result.Segments[result.Count++] = { .Kind = SegmentKind::Literal,
                .Text = { static_cast<UInt16>(literalStart), static_cast<UInt16>(end - literalStart) },
                .Directive = violet::Nothing,
                .Depth = depth };
        }

        inLiteral = false;
        return { };
    };

    while (in < len) {
        if (input[in] != '%' || in + 1 >= len) {
            if (!inLiteral) {
                literalStart = in;
                inLiteral = true;
            }

            ++in;
            continue;
        }

        VIOLET_TRY_VOID(flushLiteral(in));

        auto rest = input.substr(in + 1);
        auto [kwLen, kwKind] = tryMatchKeyword(rest);

        auto kind = static_cast<SegmentKind>(std::numeric_limits<std::underlying_type_t<SegmentKind>>::max());
        if (kwLen > 0) {
            kind = kwKind;
            in += 1 + kwLen;
        } else {
            char spec = input[in + 1];
            in += 2;

            switch (spec) {
            case 'L':
                kind = SegmentKind::Level;
                break;

            case 'm':
                kind = SegmentKind::Message;
                break;

            case 'T':
                kind = SegmentKind::ThreadId;
                break;

            case 't':
                kind = SegmentKind::Timestamp;
                break;

            case 'n':
                kind = SegmentKind::Logger;
                break;

            case 'f':
                kind = SegmentKind::File;
                break;

            case 'l':
                kind = SegmentKind::LineNum;
                break;

            case 'c':
                kind = SegmentKind::ColumnNum;
                break;

            case 'F':
                kind = SegmentKind::FunctionName;
                break;

            case 'A':
                kind = SegmentKind::AllAttributes;
                break;

            case 'X':
                kind = SegmentKind::Attribute;
                break;

            case '%': {
                if (result.Count >= result.Segments.size()) {
                    return error("pattern exceeds maximum segment count (64)");
                }

                result.Segments[result.Count++] = { .Kind = SegmentKind::Literal,
                    .Text = { static_cast<UInt16>(in - 1), 1 },
                    .Directive = violet::Nothing,
                    .Depth = depth };

                literalStart = in;
                continue;
            }

            default:
                return errfmt("unknown format directive: [%{}]", spec);
            }
        }

        if (static_cast<std::underlying_type_t<SegmentKind>>(kind)
            == std::numeric_limits<std::underlying_type_t<SegmentKind>>::max()) {
            return error("unknown or unhandled segment reached");
        }

        if (!config.AllowedSegments.empty() && !config.AllowedSegments.contains(kind)) {
            return errfmt("segment [{}] is not allowed", kind);
        }

        Optional<Directive> directive;
        if (in < len && input[in] == '{') {
            UInt32 open = in + 1;
            UInt32 braceDepth = 1;
            UInt32 ending = open;

            while (ending < len && braceDepth > 0) {
                if (input[ending] == '{') {
                    ++braceDepth;
                } else if (input[ending] == '}') {
                    --braceDepth;
                }

                ++ending;
            }

            if (braceDepth != 0) {
                return error("unterminated `{' in format directive");
            }

            UInt32 close = ending - 1;
            auto body = input.substr(open, close - open);
            in = ending;

            directive = VIOLET_TRY(Directive::Parse(body, open, kind));
        }

        if (auto cap = pattern::GetSegmentCapability(kind);
            cap == pattern::SegmentCapability::SpecOnly || kind == SegmentKind::Attribute) {
            if (!directive.HasValue() || !directive->HasSpecification()) {
                if (kind == SegmentKind::Attribute) {
                    return error("%X format directive requires a key spec (e.g., `%X{request.id}`)");
                }

                if (kind == SegmentKind::ForegroundColorStart || kind == SegmentKind::BackgroundColorStart) {
                    return error("%fg/%bg requires a spec (e.g., `%fg{red}`)");
                }
            }
        }

        if (isStyleStart(kind)) {
            ++depth;
        } else if (kind == SegmentKind::StyleEnd) {
            if (depth == 0) {
                if (!config.RelaxedStyleNesting) {
                    return error("`%style:end` with no matching style directive");
                }
            } else {
                --depth;
            }
        }

        if (result.Count >= result.Segments.size()) {
            return error("pattern exceeds maximum segment count (64)");
        }

        result.Segments[result.Count++]
            = { .Kind = kind, .Text = { 0, 0 }, .Directive = VIOLET_MOVE(directive), .Depth = depth };

        literalStart = in;
    }

    VIOLET_TRY_VOID(flushLiteral(len));
    if (depth != 0 && !config.RelaxedStyleNesting) {
        return errfmt("pattern has {} unclosed style directive(s); missing `%style:end`", depth);
    }

    return result;
}

void ParsedPattern::FormatSegments(const LogRecord& record, String& out) const
{
    for (UInt i = 0; i < this->Count; ++i) {
        this->FormatSegment(this->Segments[i], record, out);
    }
}

#define __format_impl__(Name)                                                                                          \
    void ParsedPattern::VIOLET_CONCAT(Format, Name)(                                                                   \
        [[maybe_unused]] const Segment& segment, [[maybe_unused]] const LogRecord& record, String& out) const

__format_impl__(Segment)
{
#define __dispatch_format__(kind)                                                                                      \
    case SegmentKind::kind:                                                                                            \
        this->VIOLET_CONCAT(Format, kind)(segment, record, out);                                                       \
        break

#define __dispatch_format1__(kind, fn)                                                                                 \
    case SegmentKind::kind:                                                                                            \
        this->VIOLET_CONCAT(Format, fn)(segment, record, out);                                                         \
        break

    switch (segment.Kind) {
        __dispatch_format__(Literal);
        __dispatch_format__(Timestamp);
        __dispatch_format__(Message);
        __dispatch_format__(Logger);
        __dispatch_format__(File);
        __dispatch_format__(FunctionName);
        __dispatch_format1__(LineNum, Line);
        __dispatch_format1__(ColumnNum, Column);
        __dispatch_format__(ThreadId);
        __dispatch_format__(Attribute);

    default:
        return;
    }
}

#undef __dispatch_format__
#undef __dispatch_format1__

__format_impl__(Literal)
{
    auto [offset, length] = segment.Text;
    out.append(this->Pattern.substr(offset, length));
}

__format_impl__(Timestamp)
{
    Str spec;
    if (segment.Directive.HasValue() && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    if (spec.empty()) {
        spec = "%Y-%m-%d %H:%M:%S";
    }

    auto tt = std::chrono::system_clock::to_time_t(record.Timestamp);
    if (spec == "iso8601") {
        auto* gmt = std::gmtime(&tt);

        Array<char, 32> buf;
        auto length = ::strftime(buf.data(), buf.size(), "%Y-%m-%dT%H:%M:%SZ", gmt);

        out.append({ buf.data(), length });
        return;
    }

    std::tm tm{ };
    ::localtime_r(&tt, &tm);

    String specStr(spec);
    Array<char, 128> buf{ };

    auto length = ::strftime(buf.data(), buf.size(), specStr.c_str(), &tm);
    out.append({ buf.data(), length });
}

__format_impl__(Level)
{
    Str spec;
    if (segment.Directive && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    auto text = levelToString(record.Level, spec);
    if (segment.Directive && segment.Directive->HasPadding()) {
        applyPadding(text, segment.Directive.Value(), out);
    } else {
        out.append(text);
    }
}

__format_impl__(Message)
{
    out.append(record.Message);
}

__format_impl__(Logger)
{
    if (segment.Directive && segment.Directive->HasPadding()) {
        applyPadding(record.Logger, segment.Directive.Value(), out);
    } else {
        out.append(record.Logger);
    }
}

__format_impl__(File)
{
    out.append(record.Location.File);
}

__format_impl__(FunctionName)
{
    out.append(record.Location.Function);
}

__format_impl__(Line)
{
    out.append(violet::ToString(record.Location.Line));
}

__format_impl__(Column)
{
    out.append(violet::ToString(record.Location.Column));
}

__format_impl__(ThreadId)
{
    Str spec;
    if (segment.Directive && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    constexpr std::hash<std::thread::id> hasher{ };
    auto id = std::this_thread::get_id();
    if (spec == "hex") {
        out.append(std::format("0x{:x}", hasher(id)));

        return;
    }

    out.append(std::format("{:d}", hasher(id)));
}

__format_impl__(Attribute)
{
    if (!segment.Directive || !segment.Directive->HasSpecification()) {
        return;
    }

    auto key = segment.Directive->Specification(this->Pattern);
    auto entry = record.Fields.find(String(key));
    if (entry == record.Fields.end()) {
        return;
    }

    const auto& value = entry->second;
    if (auto held = value.As<bool>()) {
        out.append(*held ? "true" : "false");
    } else if (auto held = value.As<violet::Int64>()) {
        out.append(std::format("{}", *held));
    } else if (auto held = value.As<violet::UInt64>()) {
        out.append(std::format("{}", *held));
    } else if (auto held = value.As<double>()) {
        out.append(std::format("{}", *held));
    } else if (auto held = value.As<String>()) {
        out.append(*held);
    }
}
