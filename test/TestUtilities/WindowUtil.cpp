#include "WindowUtil.hpp"

// Note: No need to increase compile time by including TestUtilities/Window.hpp
#include "SFML/Window/VideoMode.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToChars.hpp"


namespace
{
////////////////////////////////////////////////////////////
template <typename T>
sf::base::String winIntToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<sf::base::SizeT>(end - buf);
    return sf::base::String{buf, len};
}

} // namespace


namespace tst
{
////////////////////////////////////////////////////////////
sf::base::String StringMaker<sf::VideoMode>::convert(const sf::VideoMode& videoMode)
{
    return winIntToString(videoMode.size.x) + sf::base::String{"x"} + winIntToString(videoMode.size.y) +
           sf::base::String{"x"} + winIntToString(videoMode.bitsPerPixel);
}

} // namespace tst
