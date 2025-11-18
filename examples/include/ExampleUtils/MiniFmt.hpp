#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"


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
constexpr char* formatArgIntoBuffer(char* const buffer, const char* bufferEnd, const T& arg)
    requires sf::base::isFloatingPoint<T> || sf::base::isIntegral<T>
{
    return sf::base::toChars(buffer, bufferEnd, arg);
}


////////////////////////////////////////////////////////////
using ErasedFn = char* (*)(char*, const char*, const void*);


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* erasedFormatArgIntoBuffer(char* buffer, const char* bufferEnd, const void* erasedArg)
{
    return formatArgIntoBuffer(buffer, bufferEnd, *static_cast<const T*>(erasedArg));
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline char* formatIntoBufferImpl(
    char*                 buffer,
    sf::base::SizeT       bufferSize,
    sf::base::StringView  formatStr,
    const void* const*    args,
    const ErasedFn*       formatters,
    const sf::base::SizeT argCount)
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
        const void* const  erasedArgs[]         = {&args...};
        constexpr ErasedFn erasedFormatArgFns[] = {&erasedFormatArgIntoBuffer<Args>...};

        return formatIntoBufferImpl(buffer, bufferSize, formatString.str, erasedArgs, erasedFormatArgFns, sizeof...(Args));
    }
}

} // namespace minifmt

// TODO P0: continue and use
