#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Abort.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsConvertible.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"


namespace minifmt
{
////////////////////////////////////////////////////////////
template <typename T>
struct NonDeduced
{
    using type = T;
};


////////////////////////////////////////////////////////////
/// \brief Per-placeholder format spec.
///
/// Grammar (subset of `std::format`):
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
///     fill      = any single character (defaults to space)
///     align     = '<' (left), '>' (right), '^' (center)
///     width     = decimal digits
///     precision = '.' followed by decimal digits (floats only)
///     type      = 'f' (float fixed) -- purely informational, behavior is the same
///
/// Numeric args default to right-alignment, strings to left-alignment.
////////////////////////////////////////////////////////////
struct FormatSpec
{
    int  width     = 0;    //!< 0 = no padding
    int  precision = -1;   //!< -1 = unspecified (use 6 for floats)
    char align     = '\0'; //!< '\0' = type default
    char fill      = ' ';
};


////////////////////////////////////////////////////////////
[[nodiscard]] consteval sf::base::SizeT countPlaceholders(const sf::base::StringView formatString)
{
    const auto size = formatString.size();

    sf::base::SizeT count = 0u;

    for (sf::base::SizeT i = 0u; i < size; ++i)
    {
        if (formatString[i] == '{')
        {
            // Escaped '{{'
            if (i + 1u < size && formatString[i + 1u] == '{')
            {
                ++i;
                continue;
            }

            // Skip past the placeholder body up to the matching '}'
            ++i;
            while (i < size && formatString[i] != '}')
                ++i;

            if (i >= size)
                throw "Invalid format string: Unclosed '{'";

            ++count;
        }
        else if (formatString[i] == '}')
        {
            if (i + 1u < size && formatString[i + 1u] == '}')
                ++i; // Escaped '}}'
            else
                throw "Invalid format string: Unescaped '}'";
        }
    }

    return count;
}


////////////////////////////////////////////////////////////
template <typename... Args>
struct [[nodiscard]] FormatString
{
    sf::base::StringView str;

    consteval FormatString(const char* const s) : str(s)
    {
        if (countPlaceholders(str) != sizeof...(Args))
            throw "Mismatch between number of '{}' and number of arguments.";
    }
};


////////////////////////////////////////////////////////////
template <typename T, typename U>
concept ConvertibleTo = SFML_BASE_IS_CONVERTIBLE(T, U);


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr char* writeRun(char* const           buffer,
                                                       const char* const     bufferEnd,
                                                       const char            c,
                                                       const sf::base::SizeT n)
{
    if (n == 0u)
        return buffer;

    if (static_cast<sf::base::SizeT>(bufferEnd - buffer) < n)
        return nullptr;

    SFML_BASE_MEMSET(buffer, c, n);
    return buffer + n;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr char* copyBytes(char* const           buffer,
                                                        const char* const     bufferEnd,
                                                        const char* const     src,
                                                        const sf::base::SizeT len)
{
    if (static_cast<sf::base::SizeT>(bufferEnd - buffer) < len)
        return nullptr;

    SFML_BASE_MEMCPY(buffer, src, len);
    return buffer + len;
}


////////////////////////////////////////////////////////////
/// \brief Copy `[src, src+len)` into `buffer`, padding to `spec.width` if narrower.
///
/// `defaultAlign` is used when `spec.align` is unset.
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr char* writePadded(
    char* const           buffer,
    const char* const     bufferEnd,
    const char* const     src,
    const sf::base::SizeT len,
    const FormatSpec&     spec,
    const char            defaultAlign)
{
    // No padding needed (no width, or content already >= width)
    if (spec.width <= 0 || static_cast<sf::base::SizeT>(spec.width) <= len)
        return copyBytes(buffer, bufferEnd, src, len);

    const auto padTotal = static_cast<sf::base::SizeT>(spec.width) - len;
    const char align    = spec.align == '\0' ? defaultAlign : spec.align;

    sf::base::SizeT padLeft  = 0u;
    sf::base::SizeT padRight = 0u;

    if (align == '<')
        padRight = padTotal;
    else if (align == '^')
    {
        padLeft  = padTotal / 2u;
        padRight = padTotal - padLeft;
    }
    else // '>' or unknown -- default to right-align
        padLeft = padTotal;

    // Single bounds check for the whole padded run, then three direct writes.
    const auto total = padLeft + len + padRight;
    if (static_cast<sf::base::SizeT>(bufferEnd - buffer) < total)
        return nullptr;

    if (padLeft != 0u)
        SFML_BASE_MEMSET(buffer, spec.fill, padLeft);

    SFML_BASE_MEMCPY(buffer + padLeft, src, len);

    if (padRight != 0u)
        SFML_BASE_MEMSET(buffer + padLeft + len, spec.fill, padRight);

    return buffer + total;
}


////////////////////////////////////////////////////////////
/// Numeric arg → temp buffer → padded copy (right-align by default).
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] constexpr char* formatArgIntoBuffer(
    char* const       buffer,
    const char* const bufferEnd,
    const T&          arg,
    const FormatSpec& spec)
    requires sf::base::isFloatingPoint<T> || sf::base::isIntegral<T>
{
    // Fast path: no width -- write directly into the destination.
    if (spec.width <= 0)
    {
        if constexpr (sf::base::isFloatingPoint<T>)
            return sf::base::toChars(buffer, bufferEnd, arg, spec.precision >= 0 ? spec.precision : 6);
        else
            return sf::base::toChars(buffer, bufferEnd, arg);
    }

    // Padded path: format into a small temp, then pad-copy.
    // Max textual size: ~24 chars for 64-bit ints, ~32 for doubles with up to 10 frac digits.
    char        temp[40];
    char* const tempEnd = temp + sizeof(temp);

    char* const numEnd = [&]
    {
        if constexpr (sf::base::isFloatingPoint<T>)
            return sf::base::toChars(temp, tempEnd, arg, spec.precision >= 0 ? spec.precision : 6);
        else
            return sf::base::toChars(temp, tempEnd, arg);
    }();

    if (numEnd == nullptr)
        return nullptr;

    return writePadded(buffer, bufferEnd, temp, static_cast<sf::base::SizeT>(numEnd - temp), spec, '>');
}


////////////////////////////////////////////////////////////
/// String-like arg (has `.data()` and `.size()`) → padded copy (left-align by default).
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] constexpr char* formatArgIntoBuffer(
    char* const       buffer,
    const char* const bufferEnd,
    const T&          arg,
    const FormatSpec& spec)
    requires requires {
        { arg.data() } -> ConvertibleTo<const char*>;
        { arg.size() } -> ConvertibleTo<sf::base::SizeT>;
    }
{
    return writePadded(buffer, bufferEnd, arg.data(), static_cast<sf::base::SizeT>(arg.size()), spec, '<');
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr char* formatArgIntoBuffer(
    char* const       buffer,
    const char* const bufferEnd,
    const char* const arg,
    const FormatSpec& spec)
{
    SFML_BASE_ASSERT_AND_ASSUME(arg != nullptr);
    return writePadded(buffer, bufferEnd, arg, SFML_BASE_STRLEN(arg), spec, '<');
}


////////////////////////////////////////////////////////////
using ErasedFormatArgIntoBufferFn = char* (*)(char*, const char*, const void*, const FormatSpec&);


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* erasedFormatArgIntoBuffer(char*             buffer,
                                                        const char*       bufferEnd,
                                                        const void*       erasedArg,
                                                        const FormatSpec& spec)
{
    return formatArgIntoBuffer(buffer, bufferEnd, *static_cast<const T*>(erasedArg), spec);
}


////////////////////////////////////////////////////////////
[[nodiscard]] char* formatIntoBufferImpl(
    char*                              buffer,
    sf::base::SizeT                    bufferSize,
    sf::base::StringView               formatStr,
    const void* const*                 args,
    const ErasedFormatArgIntoBufferFn* formatters,
    sf::base::SizeT                    argCount);


////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr char* formatIntoBuffer(char*                                                  buffer,
                                               const sf::base::SizeT                                  bufferSize,
                                               typename NonDeduced<const FormatString<Args...>>::type formatString,
                                               const Args&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        // Optimization: no args, just copy the string (handling escapes)
        return formatIntoBufferImpl(buffer, bufferSize, formatString.str, nullptr, nullptr, 0u);
    }
    else
    {
        const void* const                     erasedArgs[]         = {&args...};
        constexpr ErasedFormatArgIntoBufferFn erasedFormatArgFns[] = {&erasedFormatArgIntoBuffer<Args>...};

        return formatIntoBufferImpl(buffer, bufferSize, formatString.str, erasedArgs, erasedFormatArgFns, sizeof...(Args));
    }
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT N, typename... Args>
[[nodiscard]] constexpr char* formatIntoBuffer(char (&buffer)[N],
                                               typename NonDeduced<const FormatString<Args...>>::type formatString,
                                               const Args&... args)
{
    return formatIntoBuffer(buffer, N, formatString, args...);
}


////////////////////////////////////////////////////////////
/// \brief Sink that accepts `append(const char*, SizeT)` (e.g. `sf::base::String`, `sf::Utf8String`).
////////////////////////////////////////////////////////////
template <typename T>
concept FormatSink = requires(T& sink, const char* p, sf::base::SizeT n) { sink.append(p, n); };


////////////////////////////////////////////////////////////
/// \brief Format into any sink with an `append(const char*, SizeT)` method.
///
/// The whole result is staged in a 512-byte stack buffer and flushed in one `append`,
/// so the sink sees at most one allocation and one memcpy. Aborts if the formatted
/// output exceeds 512 bytes.
////////////////////////////////////////////////////////////
template <FormatSink Sink, typename... Args>
constexpr void formatTo(Sink& sink, typename NonDeduced<const FormatString<Args...>>::type formatString, const Args&... args)
{
    char              buffer[512];
    const auto* const endPtr = formatIntoBuffer(buffer, formatString, args...);

    if (endPtr == nullptr)
        sf::base::abort(); // Formatting failed (buffer too small)

    sink.append(buffer, static_cast<sf::base::SizeT>(endPtr - buffer));
}


////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr sf::base::String format(const typename NonDeduced<const FormatString<Args...>>::type formatString,
                                                const Args&... args)
{
    sf::base::String out;
    formatTo(out, formatString, args...);
    return out;
}


////////////////////////////////////////////////////////////
void printImpl(const char* formattedString);


////////////////////////////////////////////////////////////
template <typename... Args>
void print(const typename NonDeduced<const FormatString<Args...>>::type formatString, const Args&... args)
{
    char              buffer[512];
    const auto* const endPtr = formatIntoBuffer(buffer, formatString, args...);

    if (endPtr == nullptr)
        sf::base::abort(); // Formatting failed (buffer too small)

    printImpl(buffer);
}

} // namespace minifmt
