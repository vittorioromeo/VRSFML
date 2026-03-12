#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsConvertible.hpp"


namespace minifmt
{
////////////////////////////////////////////////////////////
template <typename T>
struct NonDeduced
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <sf::base::SizeT N>
struct [[nodiscard]] FixedString
{
    char data[N]{};

    enum : sf::base::SizeT
    {
        size = N - 1 // Exclude null terminator
    };

    consteval FixedString(const char (&arr)[N])
    {
        for (sf::base::SizeT i = 0u; i < N; ++i)
            data[i] = arr[i];
    }
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
            if (i + 1 < size && formatString[i + 1] == '}')
            {
                ++count;
                ++i; // Found {}
            }
            else if (i + 1 < size && formatString[i + 1] == '{')
            {
                ++i; // Found {{, skip
            }
            else
            {
                throw "Invalid format string: Found unclosed or invalid '{'";
            }
        }
        else if (formatString[i] == '}')
        {
            if (i + 1 < size && formatString[i + 1] == '}')
            {
                ++i; // Found }}, skip
            }
            else
            {
                throw "Invalid format string: Found unescaped '}'";
            }
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
template <typename T>
[[nodiscard, gnu::always_inline]] constexpr char* formatArgIntoBuffer(char* const buffer, const char* const bufferEnd, const T& arg)
    requires sf::base::isFloatingPoint<T> || sf::base::isIntegral<T>
{
    return sf::base::toChars(buffer, bufferEnd, arg);
}


////////////////////////////////////////////////////////////
template <typename T, typename U>
concept ConvertibleTo = SFML_BASE_IS_CONVERTIBLE(T, U);


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr char* copyStringIntoBuffer(
    char* const           buffer,
    const char* const     bufferEnd,
    const char* const     src,
    const sf::base::SizeT len)
{
    if (static_cast<sf::base::SizeT>(bufferEnd - buffer) < len)
        return nullptr;

    for (sf::base::SizeT i = 0; i < len; ++i)
        buffer[i] = src[i];

    return buffer + len;
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline]] constexpr char* formatArgIntoBuffer(char* const buffer, const char* const bufferEnd, const T& arg)
    requires requires {
        { arg.data() } -> ConvertibleTo<const char*>;
        { arg.size() } -> ConvertibleTo<sf::base::SizeT>;
    }
{
    return copyStringIntoBuffer(buffer, bufferEnd, arg.data(), static_cast<sf::base::SizeT>(arg.size()));
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr char* formatArgIntoBuffer(char* const       buffer,
                                                                             const char* const bufferEnd,
                                                                             const char* const arg)
{
    return copyStringIntoBuffer(buffer, bufferEnd, arg, SFML_BASE_STRLEN(arg));
}


////////////////////////////////////////////////////////////
using ErasedFormatArgIntoBufferFn = char* (*)(char*, const char*, const void*);


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* erasedFormatArgIntoBuffer(char* buffer, const char* bufferEnd, const void* erasedArg)
{
    return formatArgIntoBuffer(buffer, bufferEnd, *static_cast<const T*>(erasedArg));
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline char* formatIntoBufferImpl(
    char*                              buffer,
    sf::base::SizeT                    bufferSize,
    sf::base::StringView               formatStr,
    const void* const*                 args,
    const ErasedFormatArgIntoBufferFn* formatters,
    const sf::base::SizeT              argCount)
{
    const auto* const bufferEnd = buffer + bufferSize;

    const auto*       p   = formatStr.data();
    const auto* const end = p + formatStr.size();

    sf::base::SizeT argIndex = 0u;

    while (p < end)
    {
        // Check for placeholder "{}"
        if (*p == '{' && (p + 1 < end) && *(p + 1) == '}')
        {
            if (argIndex >= argCount)
                return nullptr;

            // Call the type-erased formatter
            auto* nextP = formatters[argIndex](buffer, bufferEnd, args[argIndex]);
            ++argIndex;

            if (nextP == nullptr)
                return nullptr;

            p += 2; // Skip "{}"
            buffer = nextP;
        }
        // Handle Escaped "{{" -> "{"
        else if (*p == '{' && (p + 1 < end) && *(p + 1) == '{')
        {
            if (buffer >= bufferEnd)
                return nullptr;

            *buffer++ = '{';
            p += 2;
        }
        // Handle Escaped "}}" -> "}"
        else if (*p == '}' && (p + 1 < end) && *(p + 1) == '}')
        {
            if (buffer >= bufferEnd)
                return nullptr;

            *buffer++ = '}';
            p += 2;
        }
        else
        {
            if (buffer >= bufferEnd)
                return nullptr;

            *buffer++ = *p++;
        }
    }


    if (buffer >= bufferEnd)
        return nullptr; // No room for null terminator

    *buffer = '\0';
    return buffer;
}


////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr char* formatIntoBuffer(char*                                                  buffer,
                                               const sf::base::SizeT                                  bufferSize,
                                               typename NonDeduced<const FormatString<Args...>>::type formatString,
                                               const Args&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        // Optimization: No args, just copy the string (handling escapes)
        return formatIntoBufferImpl(buffer, bufferSize, formatString.str, nullptr, nullptr, 0);
    }
    else
    {
        const void* const                     erasedArgs[]         = {&args...};
        constexpr ErasedFormatArgIntoBufferFn erasedFormatArgFns[] = {&erasedFormatArgIntoBuffer<Args>...};

        return formatIntoBufferImpl(buffer, bufferSize, formatString.str, erasedArgs, erasedFormatArgFns, sizeof...(Args));
    }
}


////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] constexpr sf::base::String format(const typename NonDeduced<const FormatString<Args...>>::type formatString,
                                                const Args&... args)
{
    enum : sf::base::SizeT
    {
        bufferSize = 512
    };

    char buffer[bufferSize];

    const auto* const endPtr = formatIntoBuffer(buffer, bufferSize, formatString, args...);

    if (endPtr == nullptr)
        sf::base::abort(); // Formatting failed (buffer too small)

    return sf::base::String(buffer, static_cast<sf::base::SizeT>(endPtr - buffer));
}

} // namespace minifmt

// TODO P0: continue and use
