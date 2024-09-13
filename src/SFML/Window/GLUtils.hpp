#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get GL integer and assert success
///
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName);

} // namespace sf::priv
