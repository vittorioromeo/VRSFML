// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/MiniFmt.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"

#include <cstdio>


namespace minifmt
{
////////////////////////////////////////////////////////////
char* formatIntoBufferImpl(char*                              buffer,
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
void printImpl(const char* formattedString)
{
    std::puts(formattedString);
}

} // namespace minifmt
