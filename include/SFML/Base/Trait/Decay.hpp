#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__decay)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_DECAY(...) __decay(__VA_ARGS__)

#else

    #include "SFML/Base/Trait/AddPointer.hpp"
    #include "SFML/Base/Trait/Conditional.hpp"
    #include "SFML/Base/Trait/IsConst.hpp"
    #include "SFML/Base/Trait/RemoveCV.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename U>
struct DecaySelector
{
    using type = Conditional<SFML_BASE_IS_CONST(const U), SFML_BASE_REMOVE_CV(U), SFML_BASE_ADD_POINTER(U)>;
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

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_DECAY(...) typename ::sf::base::priv::Decay<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using Decay = SFML_BASE_DECAY(T);

} // namespace sf::base
