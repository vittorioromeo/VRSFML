#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__decay)

    ////////////////////////////////////////////////////////////
    #define ZA_DECAY(...) __decay(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "Zancle/Trait/AddPointer.hpp"
    #include "Zancle/Trait/Conditional.hpp"
    #include "Zancle/Trait/IsConst.hpp"
    #include "Zancle/Trait/RemoveCV.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
template <typename U>
struct DecaySelector
{
    using type = Conditional<ZA_IS_CONST(const U), ZA_REMOVE_CV(U), ZA_ADD_POINTER(U)>;
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

} // namespace za::priv

    ////////////////////////////////////////////////////////////
    #define ZA_DECAY(...) typename ::za::priv::Decay<__VA_ARGS__>::type

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
using Decay = ZA_DECAY(T);

} // namespace za
