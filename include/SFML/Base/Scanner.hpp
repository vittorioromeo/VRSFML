#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/FromChars.hpp"      // IWYU pragma: export -- callers use the parse results
#include "SFML/Base/FromCharsRadix.hpp" // IWYU pragma: export
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Radix.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Concept for the byte source consumed by `Scanner`.
///
/// A source exposes a single primitive: fill up to `n` bytes into `dst`
/// and return how many were actually written. Zero means end-of-input,
/// which the scanner treats as sticky -- once the source returns zero,
/// it is never queried again.
///
////////////////////////////////////////////////////////////
template <typename T>
concept ScannerSource = requires(T& src, char* dst, SizeT n) {
    // `refill(dst, n)` writes up to `n` bytes into `dst` and returns
    // how many were actually written (0 = EOF). Convertibility to `SizeT`
    // is implicit at the call site -- no explicit `same_as` constraint
    // (matches the style of `AppendSink` in `Fmt.hpp`).
    src.refill(dst, n);
};


////////////////////////////////////////////////////////////
/// \brief In-memory `ScannerSource` backed by a `StringView`.
///
/// First `refill` call hands the whole view over via `memcpy`; subsequent
/// calls return 0. The scanner's internal staging buffer is therefore
/// only exercised once per parse.
///
////////////////////////////////////////////////////////////
struct StringSource
{
    StringView remaining;

    [[gnu::always_inline]] SizeT refill(char* const dst, const SizeT n) noexcept
    {
        const SizeT take = remaining.size() < n ? remaining.size() : n;
        if (take == 0u)
            return 0u;

        SFML_BASE_MEMCPY(dst, remaining.data(), take);
        remaining = remaining.substrByPosLen(take);
        return take;
    }
};


////////////////////////////////////////////////////////////
/// \brief Streaming text parser over an arbitrary `ScannerSource`.
///
/// All parsing operations return `bool` -- `true` on success, `false` on
/// failure (EOF, malformed input, overflow). There is no sticky error
/// flag; callers chain with `if (!scanner.readX(...)) break;` (or
/// equivalent) instead of `iostream`-style `while (in >> ...)`.
///
/// Single-pass: every byte handed back by `refill` is consumed in order,
/// with no rewind. The scanner keeps a small staging buffer; large inputs
/// trigger multiple `refill` calls, so memory use is bounded regardless
/// of source size.
///
////////////////////////////////////////////////////////////
template <ScannerSource Source>
class Scanner
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Size of the staging buffer. Single full HTTP / FTP lines
    /// fit comfortably; pathological long lines just trigger extra
    /// `refill` calls. Sized to match `fmtScratchSize`.
    ////////////////////////////////////////////////////////////
    static constexpr SizeT bufferSize = 512u;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit Scanner(Source src) noexcept : m_source{SFML_BASE_MOVE(src)}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Total bytes the scanner has handed to the caller (across
    /// all `readX` operations). Useful for "resume from cursor"
    /// patterns (e.g. FTP's partial-buffer continuation).
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT bytesConsumed() const noexcept
    {
        return m_consumed;
    }


    ////////////////////////////////////////////////////////////
    /// \brief `true` when no more bytes are available from the source
    /// and the staging buffer is empty.
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool atEnd()
    {
        return !ensureNonEmpty();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Read the next byte verbatim (no whitespace skip).
    /// Returns `false` at end-of-input.
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool readChar(char& out)
    {
        if (!ensureNonEmpty())
            return false;

        out = m_buf[m_pos++];
        ++m_consumed;
        return true;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Skip whitespace, then read a maximal run of non-whitespace
    /// characters into `out`. Returns `false` if only whitespace remains.
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool readToken(String& out)
    {
        skipWhitespace();
        if (atEnd())
            return false;

        out.clear();
        readUntil(out, [](char c) { return isWhitespace(c); });
        return !out.empty();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Read up to (and consume) the next `'\n'`. The `'\n'`
    /// itself is not included in `out`. Trailing `'\r'` from CRLF
    /// endings stays in `out` (callers strip it if they need to).
    ///
    /// Returns `false` only when nothing was read because the scanner
    /// is already at end-of-input.
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool readLine(String& out)
    {
        if (atEnd())
            return false;

        out.clear();
        const bool sawNewline = readUntil(out, [](char c) { return c == '\n'; });

        if (sawNewline)
        {
            // Consume the '\n' that terminated the loop.
            ++m_pos;
            ++m_consumed;
        }

        return true;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Skip whitespace, then parse a decimal integer via
    /// `fromChars`. Accepts an optional leading sign for signed `T`.
    /// Returns `false` on EOF, malformed input, or overflow.
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool readInt(T& out)
        requires isIntegral<T>
    {
        skipWhitespace();

        // Gather sign + digits into a local buffer, then hand to `fromChars`.
        // 24 chars covers signed/unsigned up to 64 bits (worst case
        // "-9223372036854775808" = 20 chars).
        char  tmp[24];
        SizeT n = 0u;

        if (!ensureNonEmpty())
            return false;

        if (m_buf[m_pos] == '+' || m_buf[m_pos] == '-')
            tmp[n++] = consume();

        while (n < sizeof(tmp) && ensureNonEmpty() && isDecDigit(m_buf[m_pos]))
            tmp[n++] = consume();

        const auto r = fromChars(tmp, tmp + n, out);
        return r.ec == FromCharsError::None && r.ptr != tmp;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Skip whitespace, then parse an unsigned integer in the
    /// given `radix`. No sign accepted -- matches `fromCharsRadix`.
    /// Returns `false` on EOF, malformed input, or overflow.
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool readIntRadix(T& out, const Radix radix)
        requires isIntegral<T>
    {
        skipWhitespace();

        // 64 chars covers a 64-bit value in `Radix::Bin`, the densest case.
        char  tmp[64];
        SizeT n = 0u;

        // Only gather characters that are valid digits for the chosen radix.
        // Without this guard, e.g. `Radix::Oct` would silently consume an '8'
        // before `fromCharsRadix` rejected it -- the byte would be gone from
        // the scanner even though it wasn't part of the parsed value.
        const auto base = static_cast<unsigned>(radix);

        while (n < sizeof(tmp) && ensureNonEmpty() && isDigitForRadix(m_buf[m_pos], base))
            tmp[n++] = consume();

        const auto r = fromCharsRadix(tmp, tmp + n, out, radix);
        return r.ec == FromCharsError::None && r.ptr != tmp;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Consume bytes until either `delim` is seen (and consumed)
    /// or `maxCount` bytes have been skipped, whichever happens first.
    /// Mirrors `std::istream::ignore` semantics.
    ////////////////////////////////////////////////////////////
    void skipPast(const char delim, const SizeT maxCount = static_cast<SizeT>(-1))
    {
        SizeT skipped = 0u;
        while (skipped < maxCount && ensureNonEmpty())
        {
            const char c = consume();
            ++skipped;
            if (c == delim)
                return;
        }
    }

private:
    ////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static constexpr bool isWhitespace(const char c) noexcept
    {
        // Matches the default `std::isspace` set used by `iostream::operator>>`.
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
    }


    [[nodiscard, gnu::always_inline]] static constexpr bool isDecDigit(const char c) noexcept
    {
        return c >= '0' && c <= '9';
    }


    [[nodiscard, gnu::always_inline]] static constexpr bool isDigitForRadix(const char c, const unsigned base) noexcept
    {
        if (c >= '0' && c <= '9')
            return static_cast<unsigned>(c - '0') < base;

        if (c >= 'a' && c <= 'f')
            return static_cast<unsigned>(c - 'a' + 10) < base;

        if (c >= 'A' && c <= 'F')
            return static_cast<unsigned>(c - 'A' + 10) < base;

        return false;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Ensure the staging buffer has at least one unread byte.
    /// Pulls from the source if necessary. Returns `false` only when
    /// the source is exhausted.
    ////////////////////////////////////////////////////////////
    bool ensureNonEmpty()
    {
        if (m_pos < m_end) [[likely]]
            return true;

        m_pos = 0u;
        m_end = m_source.refill(m_buf, bufferSize);
        return m_end > 0u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Consume the byte at `m_buf[m_pos]`. Pre: `m_pos < m_end`.
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] char consume() noexcept
    {
        const char c = m_buf[m_pos++];
        ++m_consumed;
        return c;
    }


    void skipWhitespace()
    {
        while (ensureNonEmpty() && isWhitespace(m_buf[m_pos]))
            consume();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Accumulate bytes into `out` until `stop(c)` returns true
    /// or the source is exhausted. The stopping byte is **not**
    /// consumed; the caller decides whether to advance past it.
    ///
    /// Returns `true` if a stop byte was found, `false` if the source
    /// ran out first.
    ////////////////////////////////////////////////////////////
    template <typename Pred>
    bool readUntil(String& out, Pred&& stop)
    {
        while (ensureNonEmpty())
        {
            // Find the run of bytes inside the current buffer slice that
            // don't trigger `stop`. Appending in one batch is cheaper than
            // per-byte `out.append(&c, 1)`.
            const SizeT runStart = m_pos;
            while (m_pos < m_end && !stop(m_buf[m_pos]))
                ++m_pos;

            const SizeT runLen = m_pos - runStart;
            if (runLen != 0u)
            {
                out.append(m_buf + runStart, runLen);
                m_consumed += runLen;
            }

            if (m_pos < m_end)
                return true; // Hit a stop byte; leave it in the buffer.
        }

        return false; // EOF without a stop byte.
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Source m_source;
    char   m_buf[bufferSize]{}; //!< Staging buffer; only the `[0, m_end)` slice is meaningful.
    SizeT  m_pos      = 0u;     //!< Read cursor inside `m_buf`.
    SizeT  m_end      = 0u;     //!< Logical end of valid bytes inside `m_buf`.
    SizeT  m_consumed = 0u;     //!< Total bytes handed back to the caller.
};

} // namespace sf::base
