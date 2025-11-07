#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/String.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr bool isWhitespace(const char c) noexcept
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename StreamLike>
StreamLike& operator<<(StreamLike& stream, const String& s)
    requires(requires { stream.write(s.data(), static_cast<long>(s.size())); })
{
    stream.write(s.data(), static_cast<long>(s.size()));
    return stream;
}


////////////////////////////////////////////////////////////
/// \brief Extracts a word from a stream into a string.
///
/// This operator mimics the behavior of std::istream >> std::string.
/// It first skips any leading whitespace, then reads characters
/// into the string until the next whitespace character or the end
/// of the stream is reached.
///
/// \param stream The input stream to extract from.
/// \param s      The string to store the extracted word.
/// \return The stream object.
///
////////////////////////////////////////////////////////////
template <typename StreamLike>
StreamLike& operator>>(StreamLike& stream, String& s)
    // This requires clause assumes a more standard stream interface than the original TODO.
    // `peek()` allows looking at the next character without consuming it.
    // `get()` consumes the next character.
    // We assume they return an int-like type where -1 (or a static EOF value) signals end-of-stream.
    requires(requires(StreamLike& str) {
        { str.peek() };
        { str.get() };
    })
{
    s.clear();

    // A sentry object would typically be used here to manage stream state,
    // but for a basic implementation, we handle it directly.

    // 1. Skip leading whitespace
    int c = stream.peek();
    while (c != -1 && priv::isWhitespace(static_cast<char>(c)))
    {
        stream.get(); // Consume the whitespace character
        c = stream.peek();
    }

    // 2. Read non-whitespace characters until the next whitespace or EOF
    if (c != -1)
    {
        while (c != -1 && !priv::isWhitespace(static_cast<char>(c)))
        {
            s.pushBack(static_cast<char>(stream.get())); // Consume and append the character
            c = stream.peek();
        }
    }

    // The stream's internal state (e.g., failbit, eofbit) should be set by
    // the stream's own get/peek methods upon failure or reaching the end.
    // This operator simply reacts to the values they return.

    return stream;
}

} // namespace sf::base
