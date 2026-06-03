#include "Tst/Detail/StringifyValue.hpp"
#include "WindowUtil.hpp"

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


namespace sf
{
////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::VideoMode& videoMode) noexcept
{
    const sf::base::String s = winIntToString(videoMode.size.x) + sf::base::String{"x"} + winIntToString(videoMode.size.y) +
                               sf::base::String{"x"} + winIntToString(videoMode.bitsPerPixel);

    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace sf
