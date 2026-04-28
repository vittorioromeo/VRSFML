#include "WindowUtil.hpp"

// Note: No need to increase compile time by including TestUtilities/Window.hpp
#include "SFML/Window/VideoMode.hpp"

#include "SFML/Base/ToChars.hpp"


namespace
{
////////////////////////////////////////////////////////////
template <typename T>
doctest::String intToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<doctest::String::size_type>(end - buf);
    return {buf, len};
}

} // namespace


namespace doctest
{
////////////////////////////////////////////////////////////
String StringMaker<sf::VideoMode>::convert(const sf::VideoMode& videoMode)
{
    return intToString(videoMode.size.x) + "x" + intToString(videoMode.size.y) + "x" + intToString(videoMode.bitsPerPixel);
}

} // namespace doctest
