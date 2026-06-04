#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__decay)

    ////////////////////////////////////////////////////////////
    #define ZB_DECAY(...) __decay(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "ZancleBase/Trait/AddPointer.hpp"
    #include "ZancleBase/Trait/Conditional.hpp"
    #include "ZancleBase/Trait/IsConst.hpp"
    #include "ZancleBase/Trait/RemoveCV.hpp"


namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename U>
struct DecaySelector
{
    using type = Conditional<ZB_IS_CONST(const U), ZB_REMOVE_CV(U), ZB_ADD_POINTER(U)>;
};


////////////////////////////////////////////////////////////
template <typename U, auto N>
struct DecaySelector<U[N]>
{
    using type = U*;
};


////////////////////////////////////////////////////////////
template <typename U>
struct DecaySelector<U[]>
{
    using type = U*;
};


////////////////////////////////////////////////////////////
template <typename T>
struct Decay
{
    using type = typename DecaySelector<T>::type;
};


////////////////////////////////////////////////////////////
template <typename T>
struct Decay<T&>
{
    using type = typename DecaySelector<T>::type;
};


////////////////////////////////////////////////////////////
template <typename T>
struct Decay<T&&>
{
    using type = typename DecaySelector<T>::type;
};

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_DECAY(...) typename ::zb::priv::Decay<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using Decay = ZB_DECAY(T);

} // namespace zb
