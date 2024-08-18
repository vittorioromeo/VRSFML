#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector3.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////

template class sf::Vector3<float>;
template class sf::Vector3<double>;
template class sf::Vector3<long double>;

#define SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(type)                      \
    template type              sf::Vector3<type>::lengthSq() const;                   \
    template type              sf::Vector3<type>::dot(const Vector3& rhs) const;      \
    template sf::Vector3<type> sf::Vector3<type>::cross(const Vector3& rhs) const;    \
    template sf::Vector3<type> sf::Vector3<type>::cwiseMul(const Vector3& rhs) const; \
    template sf::Vector3<type> sf::Vector3<type>::cwiseDiv(const Vector3& rhs) const;

SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(bool)
SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
