#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class sf::Vector2<float>;
template class sf::Vector2<double>;
template class sf::Vector2<long double>;

#define SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(type)                   \
    template type              sf::Vector2<type>::lengthSquared() const;           \
    template sf::Vector2<type> sf::Vector2<type>::perpendicular() const;           \
    template type              sf::Vector2<type>::dot(Vector2) const;              \
    template type              sf::Vector2<type>::cross(Vector2) const;            \
    template sf::Vector2<type> sf::Vector2<type>::componentWiseMul(Vector2) const; \
    template sf::Vector2<type> sf::Vector2<type>::componentWiseDiv(Vector2) const;

SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
