#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/StringView.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename StreamLike>
StreamLike& operator<<(StreamLike& stream, const StringView& stringView)
    requires(requires { stream.write(stringView.theData, static_cast<long>(stringView.theSize)); })
{
    stream.write(stringView.theData, static_cast<long>(stringView.theSize));
    return stream;
}

} // namespace sf::base
