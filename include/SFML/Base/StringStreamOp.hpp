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
/// \brief Stream-insertion operator for `sf::base::String`
///
/// Writes the entire string to any stream-like type that exposes a
/// `write(const char*, long)` member, mirroring the behavior of
/// `std::ostream << std::string`.
///
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
    requires(requires(StreamLike& str) {
        { str.peek() };
        { str.get() };
    })
{
    s.clear();

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

    return stream;
}

} // namespace sf::base
