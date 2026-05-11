// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/MiniFmt.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"

#include <cstdio>


namespace minifmt
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
/// Grammar (matches `FormatSpec` doc in the header):
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
////////////////////////////////////////////////////////////
constexpr void parseFormatSpec(const char*& p, const char* const end, FormatSpec& spec)
{
    // [[fill]align] -- fill is any char followed by an align char; or just align alone.
    if (p + 1 < end && isAlignChar(p[1]))
    {
        spec.fill  = *p++;
        spec.align = *p++;
    }
    else if (p < end && isAlignChar(*p))
    {
        spec.align = *p++;
    }

    // width
    while (p < end && isDigit(*p))
    {
        spec.width = spec.width * 10 + (*p - '0');
        SFML_BASE_ASSERT(spec.width < 65'536); // sane bound; guards against signed-int overflow UB
        ++p;
    }

    // .precision
    if (p < end && *p == '.')
    {
        ++p;
        spec.precision = 0;
        while (p < end && isDigit(*p))
        {
            spec.precision = spec.precision * 10 + (*p - '0');
            SFML_BASE_ASSERT(spec.precision < 1024); // `toChars` clamps to 10 anyway; cap well below
            ++p;
        }
    }

    // type -- currently only 'f' is meaningful (and it's a no-op tag, since
    // float formatting always uses fixed notation). We accept and skip any
    // single trailing non-'}' char for forward compat.
    if (p < end && *p != '}')
        ++p;
}

} // namespace


////////////////////////////////////////////////////////////
char* formatIntoBufferImpl(char*                              buffer,
                           const sf::base::SizeT              bufferSize,
                           const sf::base::StringView         formatStr,
                           const void* const* const           args,
                           const ErasedFormatArgIntoBufferFn* formatters,
                           const sf::base::SizeT              argCount)
{
    const auto* const bufferEnd = buffer + bufferSize;

    const auto*       p   = formatStr.data();
    const auto* const end = p + formatStr.size();

    sf::base::SizeT argIndex = 0u;

    while (p < end)
    {
        const char c = *p;

        if (c == '{')
        {
            // Escaped '{{' -> '{'
            if (p + 1 < end && p[1] == '{')
            {
                if (buffer >= bufferEnd)
                    return nullptr;

                *buffer++ = '{';
                p += 2;
                continue;
            }

            // Placeholder
            if (argIndex >= argCount)
                return nullptr;

            ++p; // skip '{'

            FormatSpec spec;
            if (p < end && *p == ':')
            {
                ++p; // skip ':'
                parseFormatSpec(p, end, spec);
            }

            if (p >= end || *p != '}')
                return nullptr; // malformed placeholder

            char* const nextP = formatters[argIndex](buffer, bufferEnd, args[argIndex], spec);
            if (nextP == nullptr)
                return nullptr;

            ++argIndex;
            buffer = nextP;
            ++p; // skip '}'
        }
        else if (c == '}')
        {
            // Escaped '}}' -> '}'
            if (p + 1 < end && p[1] == '}')
            {
                if (buffer >= bufferEnd)
                    return nullptr;

                *buffer++ = '}';
                p += 2;
            }
            else
            {
                return nullptr; // unescaped '}' (consteval should have caught this)
            }
        }
        else
        {
            if (buffer >= bufferEnd)
                return nullptr;

            *buffer++ = c;
            ++p;
        }
    }

    if (buffer >= bufferEnd)
        return nullptr; // no room for null terminator

    *buffer = '\0';
    return buffer;
}


////////////////////////////////////////////////////////////
void printImpl(const char* formattedString)
{
    std::puts(formattedString);
}

} // namespace minifmt
