#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__add_lvalue_reference)

    ////////////////////////////////////////////////////////////
    #define ZB_ADD_LVALUE_REFERENCE(...) __add_lvalue_reference(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "ZancleBase/Trait/VoidT.hpp"


namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename T, typename = void>
struct AddLvalueReferenceImpl
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddLvalueReferenceImpl<T, VoidT<T&>>
{
    using type = T&;
};

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_ADD_LVALUE_REFERENCE(...) typename ::zb::priv::AddLvalueReferenceImpl<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using AddLvalueReference = ZB_ADD_LVALUE_REFERENCE(T);

} // namespace zb
