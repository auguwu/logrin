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

#include <logrin/Formatter.h>
#include <logrin/Formatter/Pattern/Parser.h>
#include <logrin/LogRecord.h>
#include <violet/Container/Optional.h>
#include <violet/Support/Terminal.h>

namespace logrin::formatter {

/// [`Formatter`] driven by a parsed pattern string.
///
/// **Pattern** iterates a pre-parsed sequence of [`Segment`][logrin::formatter::pattern::Segent]s,
/// resolving each directive against the provided [`LogRecord`] and the current
/// terminal capabilities. Style directives emit ANSI escape sequences via
/// **Noelware.Violet**'s terminal framework when colour output is enabled.
struct LOGRIN_API Pattern final: public Formatter {
    struct LOGRIN_API Config final {
        /// Force-toggle colour output. If [`violet::Nothing`], the formatter
        /// defers to [`violet::terminal::ColoursEnabled()`].
        violet::Optional<bool> UseColours;

        /// Custom level-to-colour mapping for `%levelColor`. If not provided,
        /// a built-in mapping is used.
        violet::Optional<violet::Array<violet::Pair<LogLevel, violet::terminal::RGB>, 6>> LevelColorMap;

        /// Prefix and suffix markers for `%A` (all attributes).
        /// Default: `{"[", "]"}`.
        ///
        /// Each marker is itself parsed as a pattern, so it may contain
        /// style directives (e.g., `%bold`/`%style:end`).
        violet::Pair<violet::Str, violet::Str> Markers = { "[", "]" };

        /// Key-value delimiter for attributes. Default: `"="`.
        violet::Str Delimiter = "=";
    };

    /// Parses a pattern string and constructs a `Pattern` formatter.
    static auto Parse(violet::Str input, Config config = { .Markers = { "[", "]" }, .Delimiter = "=" })
        -> violet::Result<Pattern, pattern::Error>;

    /// Returns a [`Pattern`] that resembles Azalia's output
    ///
    /// See:
    /// <https://github.com/Noelware/azalia/blob/61ab79859a014cd7c2c45bbdd9d4925a6a896bc0/crates/log/src/writers/default.rs>
    static auto Azalia() -> Pattern;

    /// @see logrin::Formatter::Format(const logrin::LogRecord&)
    [[nodiscard]] auto Format(const LogRecord& record) const -> violet::String override;

private:
    VIOLET_EXPLICIT Pattern(pattern::ParsedPattern parsed, Config config,
        violet::Optional<pattern::ParsedPattern> prefix, violet::Optional<pattern::ParsedPattern> suffix) noexcept
        : n_parsed(VIOLET_MOVE(parsed))
        , n_config(VIOLET_MOVE(config))
        , n_prefixMarker(VIOLET_MOVE(prefix))
        , n_suffixMarker(VIOLET_MOVE(suffix))
    {
    }

    /// Walks `pattern`'s segments and dispatches each one through this formatter.
    void formatPattern(const pattern::ParsedPattern& pattern, const LogRecord& record, violet::String& out) const;

    /// Formats a single segment, dispatching content directives to `pattern`'s
    /// own [`pattern::ParsedPattern::Format*`] methods and handling style/colour/
    /// attribute directives locally.
    void formatSegment(const pattern::Segment& segment, const pattern::ParsedPattern& pattern, const LogRecord& record,
        violet::String& out) const;

    void formatAttribute(const pattern::Segment& segment, const pattern::ParsedPattern& pattern,
        const LogRecord& record, violet::String& out) const;
    void formatAllAttributes(const LogRecord& record, violet::String& out) const;

    void emitColour(const pattern::Segment& segment, const pattern::ParsedPattern& pattern, bool foreground,
        violet::String& out) const;
    void emitLevelColour(const pattern::Segment& segment, const pattern::ParsedPattern& pattern,
        const LogRecord& record, violet::String& out) const;

    void emitStyle(pattern::SegmentKind kind, violet::String& out) const;
    void emitStyleEnd(violet::String& out) const;

    pattern::ParsedPattern n_parsed;
    Config n_config;
    violet::Optional<pattern::ParsedPattern> n_prefixMarker;
    violet::Optional<pattern::ParsedPattern> n_suffixMarker;
};

} // namespace logrin::formatter
