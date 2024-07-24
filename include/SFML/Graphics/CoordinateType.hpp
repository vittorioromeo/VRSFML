#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

namespace sf
{

////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Types of texture coordinates that can be used for rendering
///
/// \see sf::Texture::bind
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] CoordinateType
{
    Normalized, //!< Texture coordinates in range [0 .. 1]
    Pixels      //!< Texture coordinates in range [0 .. size]
};

} // namespace sf
