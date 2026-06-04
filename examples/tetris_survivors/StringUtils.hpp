#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
[[nodiscard]] inline zb::String wrapText(const zb::StringView text, const zb::SizeT maxLineLength)
{
    zb::String result; // Use a single local variable for NRVO

    // Handle trivial cases
    if (text.empty() || maxLineLength == 0)
        return result;

    result.reserve(text.size());

    zb::String currentLine;
    zb::SizeT  currentPos = 0;

    // Define our whitespace characters
    constexpr zb::StringView whitespace = " \t\n\r";

    while (currentPos < text.size())
    {
        // 1. Find the start of the next word (skip leading whitespace)
        const zb::SizeT wordStart = text.findFirstNotOf(whitespace, currentPos);

        // If no more words are found, we're done
        if (wordStart == zb::StringView::nPos)
            break;

        // 2. Find the end of the word
        zb::SizeT wordEnd = text.findFirstOf(whitespace, wordStart);
        if (wordEnd == zb::StringView::nPos)
        {
            // This is the last word, so it extends to the end of the text
            wordEnd = text.size();
        }

        // 3. Extract the word as a string_view (no copy made here)
        const zb::StringView word = text.substrByPosLen(wordStart, wordEnd - wordStart);

        // 4. Decide where to place the word
        if (currentLine.empty())
        {
            // This is the first word of a new line
            currentLine.assign(word.data(), word.size());
        }
        // Check if adding a space and the next word would exceed the max length
        else if (currentLine.size() + 1 + word.size() <= maxLineLength)
        {
            // The word fits on the current line
            currentLine += ' ';
            currentLine.append(word.data(), word.size());
        }
        else
        {
            // The word does not fit. Finalize the current line.
            result.append(currentLine);
            result += '\n';

            // The new word starts the next line.
            currentLine.assign(word.data(), word.size());
        }

        // 5. Advance our position in the input string_view
        currentPos = wordEnd;
    }

    // Append the last line that was being built
    if (!currentLine.empty())
        result.append(currentLine);

    return result;
}

} // namespace tsurv
