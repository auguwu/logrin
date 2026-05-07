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

#include <logrin/Formatter/Pattern.h>

#include <violet/Strings.h>
#include <violet/Support/Terminal.h>

using logrin::AttributeValue;
using logrin::LogLevel;
using logrin::LogRecord;
using logrin::formatter::Pattern;
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
using violet::UInt8;
using violet::terminal::RGB;
using violet::terminal::Style;

#define error(message) ::violet::Err<::logrin::formatter::pattern::Error>(::violet::String(message))
#define errfmt(fmt, ...)                                                                                               \
    ::violet::Err<::logrin::formatter::pattern::Error>(::std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#if VIOLET_REQUIRE_STL(202302L)
#define strContains(input, ch) (input.contains(ch))
#else
#define strContains(input, ch) (input.find(ch) != Str::npos)
#endif

namespace {

using LevelColorPalette = Array<Pair<LogLevel, RGB>, 6>;

constexpr LevelColorPalette kDefaultLevelColorMap = {
    Pair<LogLevel, RGB>{ LogLevel::Trace, { 102, 178, 178 } },
    Pair<LogLevel, RGB>{ LogLevel::Debug, { 162, 224, 162 } },
    Pair<LogLevel, RGB>{ LogLevel::Info, { 48, 48, 105 } },
    Pair<LogLevel, RGB>{ LogLevel::Warning, { 239, 239, 184 } },
    Pair<LogLevel, RGB>{ LogLevel::Error, { 193, 50, 50 } },
    Pair<LogLevel, RGB>{ LogLevel::Fatal, { 193, 50, 50 } },
};

constexpr LevelColorPalette kAzaliaLevelColorMap = {
    Pair<LogLevel, RGB>{ LogLevel::Trace, { 163, 182, 138 } },
    Pair<LogLevel, RGB>{ LogLevel::Debug, { 148, 224, 232 } },
    Pair<LogLevel, RGB>{ LogLevel::Info, { 178, 157, 243 } },
    Pair<LogLevel, RGB>{ LogLevel::Warning, { 243, 243, 134 } },
    Pair<LogLevel, RGB>{ LogLevel::Error, { 153, 75, 104 } },
    Pair<LogLevel, RGB>{ LogLevel::Fatal, { 153, 75, 104 } },
};

struct NamedColor final {
    Str Name;
    RGB Rgb;
};

constexpr Array<NamedColor, 148> kCSSColors = {
    NamedColor{ .Name = "aliceblue", .Rgb = { 240, 248, 255 } },
    { .Name = "antiquewhite", .Rgb = { 250, 235, 215 } },
    { .Name = "aqua", .Rgb = { 0, 255, 255 } },
    { .Name = "aquamarine", .Rgb = { 127, 255, 212 } },
    { .Name = "azure", .Rgb = { 240, 255, 255 } },
    { .Name = "beige", .Rgb = { 245, 245, 220 } },
    { .Name = "bisque", .Rgb = { 255, 228, 196 } },
    { .Name = "black", .Rgb = { 0, 0, 0 } },
    { .Name = "blanchedalmond", .Rgb = { 255, 235, 205 } },
    { .Name = "blue", .Rgb = { 0, 0, 255 } },
    { .Name = "blueviolet", .Rgb = { 138, 43, 226 } },
    { .Name = "brown", .Rgb = { 165, 42, 42 } },
    { .Name = "burlywood", .Rgb = { 222, 184, 135 } },
    { .Name = "cadetblue", .Rgb = { 95, 158, 160 } },
    { .Name = "chartreuse", .Rgb = { 127, 255, 0 } },
    { .Name = "chocolate", .Rgb = { 210, 105, 30 } },
    { .Name = "coral", .Rgb = { 255, 127, 80 } },
    { .Name = "cornflowerblue", .Rgb = { 100, 149, 237 } },
    { .Name = "cornsilk", .Rgb = { 255, 248, 220 } },
    { .Name = "crimson", .Rgb = { 220, 20, 60 } },
    { .Name = "cyan", .Rgb = { 0, 255, 255 } },
    { .Name = "darkblue", .Rgb = { 0, 0, 139 } },
    { .Name = "darkcyan", .Rgb = { 0, 139, 139 } },
    { .Name = "darkgoldenrod", .Rgb = { 184, 134, 11 } },
    { .Name = "darkgray", .Rgb = { 169, 169, 169 } },
    { .Name = "darkgreen", .Rgb = { 0, 100, 0 } },
    { .Name = "darkgrey", .Rgb = { 169, 169, 169 } },
    { .Name = "darkkhaki", .Rgb = { 189, 183, 107 } },
    { .Name = "darkmagenta", .Rgb = { 139, 0, 139 } },
    { .Name = "darkolivegreen", .Rgb = { 85, 107, 47 } },
    { .Name = "darkorange", .Rgb = { 255, 140, 0 } },
    { .Name = "darkorchid", .Rgb = { 153, 50, 204 } },
    { .Name = "darkred", .Rgb = { 139, 0, 0 } },
    { .Name = "darksalmon", .Rgb = { 233, 150, 122 } },
    { .Name = "darkseagreen", .Rgb = { 143, 188, 143 } },
    { .Name = "darkslateblue", .Rgb = { 72, 61, 139 } },
    { .Name = "darkslategray", .Rgb = { 47, 79, 79 } },
    { .Name = "darkslategrey", .Rgb = { 47, 79, 79 } },
    { .Name = "darkturquoise", .Rgb = { 0, 206, 209 } },
    { .Name = "darkviolet", .Rgb = { 148, 0, 211 } },
    { .Name = "deeppink", .Rgb = { 255, 20, 147 } },
    { .Name = "deepskyblue", .Rgb = { 0, 191, 255 } },
    { .Name = "dimgray", .Rgb = { 105, 105, 105 } },
    { .Name = "dimgrey", .Rgb = { 105, 105, 105 } },
    { .Name = "dodgerblue", .Rgb = { 30, 144, 255 } },
    { .Name = "firebrick", .Rgb = { 178, 34, 34 } },
    { .Name = "floralwhite", .Rgb = { 255, 250, 240 } },
    { .Name = "forestgreen", .Rgb = { 34, 139, 34 } },
    { .Name = "fuchsia", .Rgb = { 255, 0, 255 } },
    { .Name = "gainsboro", .Rgb = { 220, 220, 220 } },
    { .Name = "ghostwhite", .Rgb = { 248, 248, 255 } },
    { .Name = "gold", .Rgb = { 255, 215, 0 } },
    { .Name = "goldenrod", .Rgb = { 218, 165, 32 } },
    { .Name = "gray", .Rgb = { 128, 128, 128 } },
    { .Name = "green", .Rgb = { 0, 128, 0 } },
    { .Name = "greenyellow", .Rgb = { 173, 255, 47 } },
    { .Name = "grey", .Rgb = { 128, 128, 128 } },
    { .Name = "honeydew", .Rgb = { 240, 255, 240 } },
    { .Name = "hotpink", .Rgb = { 255, 105, 180 } },
    { .Name = "indianred", .Rgb = { 205, 92, 92 } },
    { .Name = "indigo", .Rgb = { 75, 0, 130 } },
    { .Name = "ivory", .Rgb = { 255, 255, 240 } },
    { .Name = "khaki", .Rgb = { 240, 230, 140 } },
    { .Name = "lavender", .Rgb = { 230, 230, 250 } },
    { .Name = "lavenderblush", .Rgb = { 255, 240, 245 } },
    { .Name = "lawngreen", .Rgb = { 124, 252, 0 } },
    { .Name = "lemonchiffon", .Rgb = { 255, 250, 205 } },
    { .Name = "lightblue", .Rgb = { 173, 216, 230 } },
    { .Name = "lightcoral", .Rgb = { 240, 128, 128 } },
    { .Name = "lightcyan", .Rgb = { 224, 255, 255 } },
    { .Name = "lightgoldenrodyellow", .Rgb = { 250, 250, 210 } },
    { .Name = "lightgray", .Rgb = { 211, 211, 211 } },
    { .Name = "lightgreen", .Rgb = { 144, 238, 144 } },
    { .Name = "lightgrey", .Rgb = { 211, 211, 211 } },
    { .Name = "lightpink", .Rgb = { 255, 182, 193 } },
    { .Name = "lightsalmon", .Rgb = { 255, 160, 122 } },
    { .Name = "lightseagreen", .Rgb = { 32, 178, 170 } },
    { .Name = "lightskyblue", .Rgb = { 135, 206, 250 } },
    { .Name = "lightslategray", .Rgb = { 119, 136, 153 } },
    { .Name = "lightslategrey", .Rgb = { 119, 136, 153 } },
    { .Name = "lightsteelblue", .Rgb = { 176, 196, 222 } },
    { .Name = "lightyellow", .Rgb = { 255, 255, 224 } },
    { .Name = "lime", .Rgb = { 0, 255, 0 } },
    { .Name = "limegreen", .Rgb = { 50, 205, 50 } },
    { .Name = "linen", .Rgb = { 250, 240, 230 } },
    { .Name = "magenta", .Rgb = { 255, 0, 255 } },
    { .Name = "maroon", .Rgb = { 128, 0, 0 } },
    { .Name = "mediumaquamarine", .Rgb = { 102, 205, 170 } },
    { .Name = "mediumblue", .Rgb = { 0, 0, 205 } },
    { .Name = "mediumorchid", .Rgb = { 186, 85, 211 } },
    { .Name = "mediumpurple", .Rgb = { 147, 112, 219 } },
    { .Name = "mediumseagreen", .Rgb = { 60, 179, 113 } },
    { .Name = "mediumslateblue", .Rgb = { 123, 104, 238 } },
    { .Name = "mediumspringgreen", .Rgb = { 0, 250, 154 } },
    { .Name = "mediumturquoise", .Rgb = { 72, 209, 204 } },
    { .Name = "mediumvioletred", .Rgb = { 199, 21, 133 } },
    { .Name = "midnightblue", .Rgb = { 25, 25, 112 } },
    { .Name = "mintcream", .Rgb = { 245, 255, 250 } },
    { .Name = "mistyrose", .Rgb = { 255, 228, 225 } },
    { .Name = "moccasin", .Rgb = { 255, 228, 181 } },
    { .Name = "navajowhite", .Rgb = { 255, 222, 173 } },
    { .Name = "navy", .Rgb = { 0, 0, 128 } },
    { .Name = "oldlace", .Rgb = { 253, 245, 230 } },
    { .Name = "olive", .Rgb = { 128, 128, 0 } },
    { .Name = "olivedrab", .Rgb = { 107, 142, 35 } },
    { .Name = "orange", .Rgb = { 255, 165, 0 } },
    { .Name = "orangered", .Rgb = { 255, 69, 0 } },
    { .Name = "orchid", .Rgb = { 218, 112, 214 } },
    { .Name = "palegoldenrod", .Rgb = { 238, 232, 170 } },
    { .Name = "palegreen", .Rgb = { 152, 251, 152 } },
    { .Name = "paleturquoise", .Rgb = { 175, 238, 238 } },
    { .Name = "palevioletred", .Rgb = { 219, 112, 147 } },
    { .Name = "papayawhip", .Rgb = { 255, 239, 213 } },
    { .Name = "peachpuff", .Rgb = { 255, 218, 185 } },
    { .Name = "peru", .Rgb = { 205, 133, 63 } },
    { .Name = "pink", .Rgb = { 255, 192, 203 } },
    { .Name = "plum", .Rgb = { 221, 160, 221 } },
    { .Name = "powderblue", .Rgb = { 176, 224, 230 } },
    { .Name = "purple", .Rgb = { 128, 0, 128 } },
    { .Name = "rebeccapurple", .Rgb = { 102, 51, 153 } },
    { .Name = "red", .Rgb = { 255, 0, 0 } },
    { .Name = "rosybrown", .Rgb = { 188, 143, 143 } },
    { .Name = "royalblue", .Rgb = { 65, 105, 225 } },
    { .Name = "saddlebrown", .Rgb = { 139, 69, 19 } },
    { .Name = "salmon", .Rgb = { 250, 128, 114 } },
    { .Name = "sandybrown", .Rgb = { 244, 164, 96 } },
    { .Name = "seagreen", .Rgb = { 46, 139, 87 } },
    { .Name = "seashell", .Rgb = { 255, 245, 238 } },
    { .Name = "sienna", .Rgb = { 160, 82, 45 } },
    { .Name = "silver", .Rgb = { 192, 192, 192 } },
    { .Name = "skyblue", .Rgb = { 135, 206, 235 } },
    { .Name = "slateblue", .Rgb = { 106, 90, 205 } },
    { .Name = "slategray", .Rgb = { 112, 128, 144 } },
    { .Name = "slategrey", .Rgb = { 112, 128, 144 } },
    { .Name = "snow", .Rgb = { 255, 250, 250 } },
    { .Name = "springgreen", .Rgb = { 0, 255, 127 } },
    { .Name = "steelblue", .Rgb = { 70, 130, 180 } },
    { .Name = "tan", .Rgb = { 210, 180, 140 } },
    { .Name = "teal", .Rgb = { 0, 128, 128 } },
    { .Name = "thistle", .Rgb = { 216, 191, 216 } },
    { .Name = "tomato", .Rgb = { 255, 99, 71 } },
    { .Name = "turquoise", .Rgb = { 64, 224, 208 } },
    { .Name = "violet", .Rgb = { 238, 130, 238 } },
    { .Name = "wheat", .Rgb = { 245, 222, 179 } },
    { .Name = "white", .Rgb = { 255, 255, 255 } },
    { .Name = "whitesmoke", .Rgb = { 245, 245, 245 } },
    { .Name = "yellow", .Rgb = { 255, 255, 0 } },
    { .Name = "yellowgreen", .Rgb = { 154, 205, 50 } },
};

// Marker patterns are recursively rendered when emitting `%A`. Allowing data
// directives there would let the marker pull from the same record (and worse,
// re-enter `%A` itself), so the marker grammar is restricted to literals and
// styling-only directives.
auto markerParseConfig() -> ParseConfig
{
    return ParseConfig{ .AllowedSegments = {
                            SegmentKind::Literal,
                            SegmentKind::ForegroundColorStart,
                            SegmentKind::BackgroundColorStart,
                            SegmentKind::LevelColorStart,
                            SegmentKind::Bold,
                            SegmentKind::Dim,
                            SegmentKind::Italic,
                            SegmentKind::Underline,
                            SegmentKind::Strikethrough,
                            SegmentKind::StyleEnd,
                        },
        .RelaxedStyleNesting = true };
}

constexpr auto toLowerAscii(char ch) noexcept -> char
{
    return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch + 32) : ch;
}

constexpr auto equalsIgnoreCase(Str lhs, Str rhs) noexcept -> bool
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (UInt index = 0; index < lhs.size(); ++index) {
        if (toLowerAscii(lhs[index]) != toLowerAscii(rhs[index])) {
            return false;
        }
    }

    return true;
}

auto parseHexDigit(char ch) -> Result<UInt8, Error>
{
    if (ch >= '0' && ch <= '9') {
        return static_cast<UInt8>(ch - '0');
    }

    if (ch >= 'a' && ch <= 'f') {
        return static_cast<UInt8>(10 + (ch - 'a'));
    }

    if (ch >= 'A' && ch <= 'F') {
        return static_cast<UInt8>(10 + (ch - 'A'));
    }

    return errfmt("invalid hexadecimal digit [{}]", ch);
}

auto parseColorSpec(Str spec) -> Result<RGB, Error>
{
    auto trimmed = violet::strings::Trim(spec, [](unsigned char ch) -> bool { return ch == ' ' || ch == '\t'; });
    if (trimmed.empty()) {
        return error("empty colour spec");
    }

    // Hexadecimal: #RGB #RRGGBB
    if (trimmed.front() == '#') {
        auto hex = trimmed.substr(1);
        if (hex.size() == 6) {
            auto redHi = VIOLET_TRY(parseHexDigit(hex[0]));
            auto redLo = VIOLET_TRY(parseHexDigit(hex[1]));
            auto greenHi = VIOLET_TRY(parseHexDigit(hex[2]));
            auto greenLo = VIOLET_TRY(parseHexDigit(hex[3]));
            auto blueHi = VIOLET_TRY(parseHexDigit(hex[4]));
            auto blueLo = VIOLET_TRY(parseHexDigit(hex[5]));

            return RGB{ static_cast<UInt8>((redHi << 4) | redLo), static_cast<UInt8>((greenHi << 4) | greenLo),
                static_cast<UInt8>((blueHi << 4) | blueLo) };
        }

        if (hex.size() == 3) {
            auto red = VIOLET_TRY(parseHexDigit(hex[0]));
            auto green = VIOLET_TRY(parseHexDigit(hex[1]));
            auto blue = VIOLET_TRY(parseHexDigit(hex[2]));

            return RGB{ static_cast<UInt8>((red << 4) | red), static_cast<UInt8>((green << 4) | green),
                static_cast<UInt8>((blue << 4) | blue) };
        }

        return error("hex color must be #RGB or #RRGGBB");
    }

    if (strContains(trimmed, ",")) {
        Array<UInt8, 3> components{ };
        UInt count = 0;
        UInt cursor = 0;

        while (cursor <= trimmed.size() && count < 3) {
            while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
                ++cursor;
            }

            violet::UInt16 value = 0;
            bool hasDigits = false;
            while (cursor < trimmed.size() && trimmed[cursor] >= '0' && trimmed[cursor] <= '9') {
                value = static_cast<violet::UInt16>((value * 10) + (trimmed[cursor] - '0'));
                hasDigits = true;
                ++cursor;
            }

            if (!hasDigits) {
                return errfmt("expected numeric value in RGB spec [{}]", spec);
            }

            if (value > 255) {
                return error("RGB component out of range (0..=255)");
            }

            components[count++] = static_cast<UInt8>(value);

            while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
                ++cursor;
            }

            if (cursor < trimmed.size() && trimmed[cursor] == ',') {
                ++cursor;
            } else {
                break;
            }
        }

        if (count != 3) {
            return errfmt("RGB spec requires exactly 3 components [{}]", spec);
        }

        while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
            ++cursor;
        }

        if (cursor != trimmed.size()) {
            return error("unexpected trailing content in RGB spec");
        }

        return RGB{ components[0], components[1], components[2] };
    }

    for (const auto& named: kCSSColors) {
        if (equalsIgnoreCase(trimmed, named.Name)) {
            return named.Rgb;
        }
    }

    return errfmt("unknown color spec: [{}]", spec);
}

auto getLevelColor(LogLevel level, const LevelColorPalette& palette) noexcept -> RGB
{
    for (const auto& [lvl, rgb]: palette) {
        if (lvl == level) {
            return rgb;
        }
    }

    return { };
}

auto isColorsEnabled(const Optional<bool>& useColours) -> bool
{
    return useColours.MapOr(violet::terminal::ColoursEnabled(), [](bool value) -> bool { return value; });
}

void appendAttributeValue(const AttributeValue& value, String& out)
{
    if (auto held = value.As<bool>()) {
        out.append(*held ? "true" : "false");
        return;
    }

    if (auto held = value.As<violet::Int64>()) {
        out.append(std::format("{}", *held));
        return;
    }

    if (auto held = value.As<violet::UInt64>()) {
        out.append(std::format("{}", *held));
        return;
    }

    if (auto held = value.As<double>()) {
        out.append(std::format("{}", *held));
        return;
    }

    if (auto held = value.As<String>()) {
        out.append(*held);
        return;
    }
}

// Walks a parsed pattern and validates that every colour-bearing directive's
// spec parses cleanly. This lets [`Pattern::Format`] assume the spec is good
// and keep its signature non-fallible.
auto validateColorSpecs(const ParsedPattern& parsed) -> Result<void, Error>
{
    for (UInt index = 0; index < parsed.Count; ++index) {
        const auto& segment = parsed.Segments[index];
        const bool requiresSpec
            = segment.Kind == SegmentKind::ForegroundColorStart || segment.Kind == SegmentKind::BackgroundColorStart;

        const bool optionalSpec = segment.Kind == SegmentKind::LevelColorStart;

        if (!requiresSpec && !optionalSpec) {
            continue;
        }

        if (!segment.Directive || !segment.Directive->HasSpecification()) {
            continue;
        }

        auto spec = segment.Directive->Specification(parsed.Pattern);
        VIOLET_TRY(parseColorSpec(spec));
    }

    return { };
}

} // namespace

auto Pattern::Parse(Str input, Config config) -> Result<Pattern, Error>
{
    auto parsed = VIOLET_TRY(pattern::Parse(input));
    VIOLET_TRY_VOID(validateColorSpecs(parsed));

    Optional<ParsedPattern> prefix;
    if (!config.Markers.first.empty()) {
        auto result = pattern::Parse(config.Markers.first, markerParseConfig());
        if (result.Err()) {
            return errfmt("in marker prefix: {}", result.Error());
        }

        VIOLET_TRY_VOID(validateColorSpecs(*result));
        prefix = VIOLET_MOVE(result.Value());
    }

    Optional<ParsedPattern> suffix;
    if (!config.Markers.second.empty()) {
        auto result = pattern::Parse(config.Markers.second, markerParseConfig());
        if (result.Err()) {
            return errfmt("in marker suffix: {}", result.Error());
        }

        VIOLET_TRY_VOID(validateColorSpecs(*result));
        suffix = VIOLET_MOVE(result.Value());
    }

    return Pattern(VIOLET_MOVE(parsed), VIOLET_MOVE(config), VIOLET_MOVE(prefix), VIOLET_MOVE(suffix));
}

auto Pattern::Azalia() -> Pattern
{
    return Pattern::Parse("%fg{#868686}[%t{%B %d, %G - %H:%M:%S %p}]%style:end %levelColor[%L{>5}]%style:end "
                          "%fg{#483D8B}«%n [thread %T{hex}]»%style:end  %m    %A",
        { .UseColours = true,
            .LevelColorMap = kAzaliaLevelColorMap,
            .Markers = { "%fg{34,34,34}%bold[", "]%style:end" },
            .Delimiter = "=" })
        .Unwrap();
}

auto Pattern::Format(const LogRecord& record) const -> String
{
    String out;
    formatPattern(this->n_parsed, record, out);

    return out;
}

void Pattern::formatPattern(const ParsedPattern& pattern, const LogRecord& record, String& out) const
{
    for (UInt index = 0; index < pattern.Count; ++index) {
        formatSegment(pattern.Segments[index], pattern, record, out);
    }
}

void Pattern::formatSegment(
    const Segment& segment, const ParsedPattern& pattern, const LogRecord& record, String& out) const
{
    switch (segment.Kind) {
    case SegmentKind::Literal:
        pattern.FormatLiteral(segment, record, out);
        break;

    case SegmentKind::Timestamp:
        pattern.FormatTimestamp(segment, record, out);
        break;

    case SegmentKind::Level:
        pattern.FormatLevel(segment, record, out);
        break;

    case SegmentKind::Message:
        pattern.FormatMessage(segment, record, out);
        break;

    case SegmentKind::Logger:
        pattern.FormatLogger(segment, record, out);
        break;

    case SegmentKind::File:
        pattern.FormatFile(segment, record, out);
        break;

    case SegmentKind::FunctionName:
        pattern.FormatFunctionName(segment, record, out);
        break;

    case SegmentKind::LineNum:
        pattern.FormatLine(segment, record, out);
        break;

    case SegmentKind::ColumnNum:
        pattern.FormatColumn(segment, record, out);
        break;

    case SegmentKind::ThreadId:
        pattern.FormatThreadId(segment, record, out);
        break;

    case SegmentKind::Attribute:
        this->formatAttribute(segment, pattern, record, out);
        break;

    case SegmentKind::AllAttributes:
        this->formatAllAttributes(record, out);
        break;

    case SegmentKind::ForegroundColorStart:
        this->emitColour(segment, pattern, /*foreground=*/true, out);
        break;

    case SegmentKind::BackgroundColorStart:
        this->emitColour(segment, pattern, /*foreground=*/false, out);
        break;

    case SegmentKind::LevelColorStart:
        this->emitLevelColour(segment, pattern, record, out);
        break;

    case SegmentKind::Bold:
    case SegmentKind::Dim:
    case SegmentKind::Italic:
    case SegmentKind::Underline:
    case SegmentKind::Strikethrough:
        this->emitStyle(segment.Kind, out);
        break;

    case SegmentKind::StyleEnd:
        this->emitStyleEnd(out);
        break;
    }
}

void Pattern::formatAttribute(
    const Segment& segment, const ParsedPattern& pattern, const LogRecord& record, String& out) const
{
    if (!segment.Directive.HasValue() || !segment.Directive->HasSpecification()) {
        return;
    }

    auto key = segment.Directive->Specification(pattern.Pattern);
    auto entry = record.Fields.find(String(key));
    if (entry == record.Fields.end()) {
        return;
    }

    appendAttributeValue(entry->second, out);
}

void Pattern::formatAllAttributes(const LogRecord& record, String& out) const
{
    if (record.Fields.empty()) {
        return;
    }

    bool first = true;
    for (const auto& [key, value]: record.Fields) {
        if (!first) {
            out.append(" ");
        }

        first = false;
        if (this->n_prefixMarker) {
            formatPattern(this->n_prefixMarker.Value(), record, out);
        } else {
            out.append(this->n_config.Markers.first);
        }

        out.append(key);
        out.append(this->n_config.Delimiter);
        appendAttributeValue(value, out);

        if (this->n_suffixMarker) {
            formatPattern(this->n_suffixMarker.Value(), record, out);
        } else {
            out.append(this->n_config.Markers.second);
        }
    }
}

void Pattern::emitColour(const Segment& segment, const ParsedPattern& pattern, bool foreground, String& out) const
{
    if (!isColorsEnabled(this->n_config.UseColours)) {
        return;
    }

    if (!segment.Directive.HasValue() || !segment.Directive->HasSpecification()) {
        return;
    }

    auto spec = segment.Directive->Specification(pattern.Pattern);
    auto rgb = parseColorSpec(spec).Unwrap();
    rgb.Foreground = foreground;
    out.append(rgb.Paint());
}

void Pattern::emitLevelColour(
    const Segment& segment, const ParsedPattern& pattern, const LogRecord& record, String& out) const
{
    if (!isColorsEnabled(this->n_config.UseColours)) {
        return;
    }

    RGB rgb;
    if (segment.Directive.HasValue() && segment.Directive->HasSpecification()) {
        auto spec = segment.Directive->Specification(pattern.Pattern);
        rgb = parseColorSpec(spec).Unwrap();
    } else {
        const auto& palette
            = this->n_config.LevelColorMap.HasValue() ? this->n_config.LevelColorMap.Value() : kDefaultLevelColorMap;

        rgb = getLevelColor(record.Level, palette);
    }

    rgb.Foreground = true;
    out.append(rgb.Paint());
}

void Pattern::emitStyle(SegmentKind kind, String& out) const
{
    if (!isColorsEnabled(this->n_config.UseColours)) {
        return;
    }

    Style style;
    switch (kind) {
    case SegmentKind::Bold:
        style.Bold();
        break;

    case SegmentKind::Dim:
        style.Dim();
        break;

    case SegmentKind::Italic:
        style.Italic();
        break;

    case SegmentKind::Underline:
        style.Underline();
        break;

    case SegmentKind::Strikethrough:
        style.Strikethrough();
        break;

    default:
        return;
    }

    out.append(style.Paint());
}

void Pattern::emitStyleEnd(String& out) const
{
    if (!isColorsEnabled(this->n_config.UseColours)) {
        return;
    }

    out.append(Style{ }.Paint());
}
