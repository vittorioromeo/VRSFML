#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transformable.hpp" // NOLINT(misc-header-include-cycle)


namespace sf
{
////////////////////////////////////////////////////////////
constexpr void Transformable::scaleBy(Vector2f factor)
{
    scale.x *= factor.x;
    scale.y *= factor.y;
}


} // namespace sf
