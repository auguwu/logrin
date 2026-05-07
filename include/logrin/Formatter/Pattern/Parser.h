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

#include <logrin/Formatter/Pattern/Error.h>
#include <logrin/Formatter/Pattern/Segment.h>
#include <logrin/LogRecord.h>
#include <violet/Support/Terminal.h>

#include <unordered_set>

namespace logrin::formatter::pattern {

/// Configuration for [`Parse`] that controls the parsing itself.
struct ParseConfig final {
    /// The set of segment kinds that the parser will accept. An empty set
    /// is treated as "allow all."
    std::unordered_set<SegmentKind> AllowedSegments;

    /// When `true`, the parser does not enforce that style directives are
    /// balanced within this pattern. Used for marker prefix/suffix patterns
    /// where the closing `%style:end` lives in the *other* marker.
    bool RelaxedStyleNesting = false;
};

/// Result of a successful parsed pattern via [`Parse`].
///
/// This contains the original pattern string, decompsed segments,
/// and a count indicating how many segments were populated. Segments beyond
/// `Count` in the array are default-initialized and should not be read.
struct LOGRIN_API ParsedPattern final {
    /// The parsed segments, stored in a fixed-capacity array of at most 64
    /// entries. Only the first [`Count`] elements are valid.
    violet::Array<pattern::Segment, 64> Segments;

    /// The original pattern string that was parsed.
    violet::String Pattern;

    /// The number of valid segments in [`Segments`].
    violet::UInt Count = 0;

    /// The configuration that was passed through [`Parse`].
    ParseConfig Config;

    /// Formats all segments in the parsed pattern against the given log record,
    /// appending the fully rendered output to `out`.
    void FormatSegments(const LogRecord& record, violet::String& out) const;

    /// Dispatches a single segment to the appropriate `Format*` method based
    /// on its [`SegmentKind`], appending the result to `out`.
    void FormatSegment(const Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the literal text content of `segment` to `out` verbatim.
    void FormatLiteral(const Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Formats the timestamp from `record` according to the format specifier
    /// in `segment` and appends it to `out`.
    void FormatTimestamp(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Formats the log level from `record` and appends it to `out`.
    void FormatLevel(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the log message body from `record` to `out`.
    void FormatMessage(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the logger name from `record` to `out`.
    void FormatLogger(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the source file path from `record` to `out`.
    void FormatFile(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the source function name from `record` to `out`.
    void FormatFunctionName(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the source line number from `record` to `out`.
    void FormatLine(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the source column number from `record` to `out`.
    void FormatColumn(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Appends the thread ID of the logging thread from `record` to `out`.
    void FormatThreadId(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;

    /// Formats a single named attribute from `record`, identified by the
    /// key specified in `segment`, and appends its value to `out`.
    void FormatAttribute(const pattern::Segment& segment, const LogRecord& record, violet::String& out) const;
};

/// Parses a string pattern into a sequence of typed segments.
///
/// Takes a raw pattern string and decomposes it into at most 64
/// [`Segment`](pattern::Segment) values, validating each against the
/// optionally provided [`ParseConfig`]. The returned [`ParsedPattern`]
/// retains ownership of the original pattern string alongside the
/// parsed segments and their count.
///
/// @param input  pattern string to parse
/// @param config optional parse configuration
LOGRIN_API auto Parse(violet::Str input, const ParseConfig& config = { }) -> violet::Result<ParsedPattern, Error>;

} // namespace logrin::formatter::pattern
