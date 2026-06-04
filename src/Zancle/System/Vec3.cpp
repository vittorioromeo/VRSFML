// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Vec3.hpp"


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template struct za::Vec3<float>;
template struct za::Vec3<double>;
template struct za::Vec3<long double>;

#define ZA_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(type)                     \
    template type           za::Vec3<type>::lengthSquared() const;                   \
    template type           za::Vec3<type>::dot(const Vec3& rhs) const;              \
    template za::Vec3<type> za::Vec3<type>::cross(const Vec3& rhs) const;            \
    template za::Vec3<type> za::Vec3<type>::componentWiseMul(const Vec3& rhs) const; \
    template za::Vec3<type> za::Vec3<type>::componentWiseDiv(const Vec3& rhs) const;

ZA_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(bool)
ZA_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(int)
ZA_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
ZA_INSTANTIATE_VECTOR3_INTEGRAL_MEMBER_FUNCTIONS(::zb::SizeT)
