#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_common_type)

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T>
struct TypeIdentityImpl
{
    using type = T;
};


////////////////////////////////////////////////////////////
struct EmptyImpl
{
};


////////////////////////////////////////////////////////////
template <typename... Ts>
struct CommonTypeImpl;


////////////////////////////////////////////////////////////
template <typename... Ts>
using CommonType = typename CommonTypeImpl<Ts...>::type;


////////////////////////////////////////////////////////////
template <typename... Ts>
struct CommonTypeImpl : __builtin_common_type<CommonType, TypeIdentityImpl, EmptyImpl, Ts...>
{
};

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_COMMON_TYPE(...) ::sf::base::priv::CommonType<__VA_ARGS__>

#else

    #include "SFML/Base/DeclVal.hpp"
    #include "SFML/Base/Trait/Conditional.hpp"
    #include "SFML/Base/Trait/Decay.hpp"
    #include "SFML/Base/Trait/IsSame.hpp"
    #include "SFML/Base/Trait/RemoveCVRef.hpp"
    #include "SFML/Base/Trait/VoidT.hpp"

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, typename U>
using CondType = decltype(false ? declVal<T>() : declVal<U>());


////////////////////////////////////////////////////////////
template <typename T, typename U, typename = void>
struct CommonType3
{
};


////////////////////////////////////////////////////////////
template <typename T, typename U>
struct CommonType3<T, U, VoidT<CondType<const T&, const U&>>>
{
    using type = SFML_BASE_REMOVE_CVREF(CondType<const T&, const U&>);
};


////////////////////////////////////////////////////////////
template <typename T, typename U, typename = void>
struct CommonType2Impl : CommonType3<T, U>
{
};


////////////////////////////////////////////////////////////
template <typename T, typename U>
struct CommonType2Impl<T, U, VoidT<decltype(true ? declVal<T>() : declVal<U>())>>
{
    using type = SFML_BASE_DECAY(decltype(true ? declVal<T>() : declVal<U>()));
};


////////////////////////////////////////////////////////////
template <typename, typename = void>
struct CommonTypeImpl
{
};


////////////////////////////////////////////////////////////
template <typename... T>
struct CommonTypes;


////////////////////////////////////////////////////////////
template <typename... T>
struct CommonType;


////////////////////////////////////////////////////////////
template <typename T, typename U>
struct CommonTypeImpl<CommonTypes<T, U>, VoidT<typename CommonType<T, U>::type>>
{
    using type = typename CommonType<T, U>::type;
};


////////////////////////////////////////////////////////////
template <typename T, typename U, typename V, typename... Rest>
struct CommonTypeImpl<CommonTypes<T, U, V, Rest...>, VoidT<typename CommonType<T, U>::type>> :
    CommonTypeImpl<CommonTypes<typename CommonType<T, U>::type, V, Rest...>>
{
};


////////////////////////////////////////////////////////////
template <>
struct CommonType<>
{
};


////////////////////////////////////////////////////////////
template <typename T>
struct CommonType<T> : public CommonType<T, T>
{
};


////////////////////////////////////////////////////////////
template <typename T, typename U>
struct CommonType<T, U> :
    Conditional<SFML_BASE_IS_SAME(T, SFML_BASE_DECAY(T)) && SFML_BASE_IS_SAME(U, SFML_BASE_DECAY(U)),
                CommonType2Impl<T, U>,
                CommonType<SFML_BASE_DECAY(T), SFML_BASE_DECAY(U)>>
{
};


////////////////////////////////////////////////////////////
template <typename T, typename U, typename V, typename... Rest>
struct CommonType<T, U, V, Rest...> : CommonTypeImpl<CommonTypes<T, U, V, Rest...>>
{
};

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_COMMON_TYPE(...) typename ::sf::base::priv::CommonType<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Ts>
using CommonType = SFML_BASE_COMMON_TYPE(Ts...);

} // namespace sf::base
