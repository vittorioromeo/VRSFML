#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class sf::Vec2<float>;
template class sf::Vec2<double>;
template class sf::Vec2<long double>;

#define SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(type)          \
    template type           sf::Vec2<type>::lengthSquared() const;        \
    template sf::Vec2<type> sf::Vec2<type>::perpendicular() const;        \
    template type           sf::Vec2<type>::dot(Vec2) const;              \
    template type           sf::Vec2<type>::cross(Vec2) const;            \
    template sf::Vec2<type> sf::Vec2<type>::componentWiseMul(Vec2) const; \
    template sf::Vec2<type> sf::Vec2<type>::componentWiseDiv(Vec2) const;

SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
