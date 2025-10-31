// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec3.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class sf::Vec3<float>;
template class sf::Vec3<double>;
template class sf::Vec3<long double>;

#define SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(type)                     \
    template type           sf::Vec3<type>::lengthSquared() const;                   \
    template type           sf::Vec3<type>::dot(const Vec3& rhs) const;              \
    template sf::Vec3<type> sf::Vec3<type>::cross(const Vec3& rhs) const;            \
    template sf::Vec3<type> sf::Vec3<type>::componentWiseMul(const Vec3& rhs) const; \
    template sf::Vec3<type> sf::Vec3<type>::componentWiseDiv(const Vec3& rhs) const;

SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(bool)
SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(::sf::base::SizeT)
