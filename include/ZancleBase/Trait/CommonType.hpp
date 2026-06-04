#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_common_type)

namespace zb::priv
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

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_COMMON_TYPE(...) ::zb::priv::CommonType<__VA_ARGS__>

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "ZancleBase/DeclVal.hpp"
    #include "ZancleBase/Trait/Conditional.hpp"
    #include "ZancleBase/Trait/Decay.hpp"
    #include "ZancleBase/Trait/IsSame.hpp"
    #include "ZancleBase/Trait/RemoveCVRef.hpp"
    #include "ZancleBase/Trait/VoidT.hpp"


namespace zb::priv
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
    using type = ZB_REMOVE_CVREF(CondType<const T&, const U&>);
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
    using type = ZB_DECAY(decltype(true ? declVal<T>() : declVal<U>()));
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
    Conditional<ZB_IS_SAME(T, ZB_DECAY(T)) && ZB_IS_SAME(U, ZB_DECAY(U)),
                CommonType2Impl<T, U>,
                CommonType<ZB_DECAY(T), ZB_DECAY(U)>>
{
};


////////////////////////////////////////////////////////////
template <typename T, typename U, typename V, typename... Rest>
struct CommonType<T, U, V, Rest...> : CommonTypeImpl<CommonTypes<T, U, V, Rest...>>
{
};

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_COMMON_TYPE(...) typename ::zb::priv::CommonType<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename... Ts>
using CommonType = ZB_COMMON_TYPE(Ts...);

} // namespace zb
