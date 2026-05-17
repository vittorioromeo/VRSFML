// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/MiniFmt.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>


namespace sf::base::priv
{
namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr bool isAlignChar(const char c)
{
    return c == '<' || c == '>' || c == '^';
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr bool isDigit(const char c)
{
    return c >= '0' && c <= '9';
}


////////////////////////////////////////////////////////////
/// \brief Parse a format spec body (everything between '{:' and '}').
///
/// `p` is advanced to the position of the closing '}'. Caller verifies '}' presence.
///
/// Grammar:
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
////////////////////////////////////////////////////////////
constexpr void parseFormatSpec(const char*& p, const char* const end, FormatSpec& spec)
{
    if (p + 1 < end && isAlignChar(p[1]))
    {
        spec.fill  = *p++;
        spec.align = *p++;
    }
    else if (p < end && isAlignChar(*p))
    {
        spec.align = *p++;
    }

    while (p < end && isDigit(*p))
    {
        spec.width = spec.width * 10 + (*p - '0');
        SFML_BASE_ASSERT(spec.width < 65'536);
        ++p;
    }

    if (p < end && *p == '.')
    {
        ++p;
        spec.precision = 0;
        while (p < end && isDigit(*p))
        {
            spec.precision = spec.precision * 10 + (*p - '0');
            SFML_BASE_ASSERT(spec.precision < 1024);
            ++p;
        }
    }

    // Type tag -- accept and skip a single non-'}' char (currently 'f' is the only meaningful one).
    if (p < end && *p != '}')
        ++p;
}

} // namespace


////////////////////////////////////////////////////////////
void formatAssembleImpl(FormatSink&              sink,
                        const FmtSpan            fmtStr,
                        const void* const* const args,
                        const ErasedDispatchFn*  dispatchers,
                        const SizeT              argCount)
{
    const auto*       p   = fmtStr.data;
    const auto* const end = p + fmtStr.size;

    SizeT argIndex = 0u;

    while (p < end)
    {
        const char c = *p;

        if (c == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                sink.appendChar('{');
                p += 2;
                continue;
            }

            // Consteval validation in `FormatString` should catch arg/spec
            // mismatches at compile time. These branches are defensive (e.g.
            // for recursive `sink.format(...)` calls that already passed
            // their own consteval).
            if (argIndex >= argCount)
            {
                sink.append("<arg?>", 6u);
                ++p;
                continue;
            }

            ++p; // skip '{'

            FormatSpec spec;
            if (p < end && *p == ':')
            {
                ++p; // skip ':'
                parseFormatSpec(p, end, spec);
            }

            if (p >= end || *p != '}')
            {
                sink.append("<spec?>", 7u);
                continue;
            }

            dispatchers[argIndex](sink, args[argIndex], spec);
            ++argIndex;
            ++p; // skip '}'
        }
        else if (c == '}')
        {
            // Escaped '}}' -> '}'. Consteval forbids a bare '}', so the
            // fallback branch is defensive.
            if (p + 1 < end && p[1] == '}')
            {
                sink.appendChar('}');
                p += 2;
            }
            else
            {
                sink.appendChar('}');
                ++p;
            }
        }
        else
        {
            sink.appendChar(c);
            ++p;
        }
    }
}


////////////////////////////////////////////////////////////
void formatToHeapFallback(void* const userSink,
                          void (*appendFn)(void*, const char*, SizeT),
                          const FmtSpan                 fmtStr,
                          const void* const* const      args,
                          const ErasedDispatchFn* const dispatchers,
                          const SizeT                   argCount)
{
    String buf;
    SizeT  cap     = formatToStagingSize * 2u;
    bool   success = false;

    // Double the capacity until the format succeeds. `resizeAndOverwrite`
    // reserves exactly `cap` bytes; the lambda writes into them and reports
    // the final size (0 signals "didn't fit, please grow"). RAII keeps the
    // buffer owned by `buf`.
    while (!success)
    {
        buf.resizeAndOverwrite(cap,
                               [&](char* const data, const SizeT capacity) -> SizeT
        {
            FormatSink sink{data, data + capacity};
            formatAssembleImpl(sink, fmtStr, args, dispatchers, argCount);

            if (sink.overflowed())
                return 0u;

            success = true;
            return sink.size();
        });

        if (!success)
            cap *= 2u;
    }

    appendFn(userSink, buf.data(), buf.size());
}


////////////////////////////////////////////////////////////
void formatWriteStdout(const char* const data, const SizeT size)
{
    std::fwrite(data, 1, size, stdout);
}


////////////////////////////////////////////////////////////
void formatWriteStdoutNewline()
{
    std::fputc('\n', stdout);
}


////////////////////////////////////////////////////////////
void formatFlushStdout()
{
    std::fflush(stdout);
}


////////////////////////////////////////////////////////////
void formatWriteStderr(const char* const data, const SizeT size)
{
    std::fwrite(data, 1, size, stderr);
}


////////////////////////////////////////////////////////////
void formatWriteStderrNewline()
{
    std::fputc('\n', stderr);
}


////////////////////////////////////////////////////////////
void formatFlushStderr()
{
    std::fflush(stderr);
}

} // namespace sf::base::priv
