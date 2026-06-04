#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__has_virtual_destructor)

    ////////////////////////////////////////////////////////////
    #define ZB_HAS_VIRTUAL_DESTRUCTOR(...) __has_virtual_destructor(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_HAS_VIRTUAL_DESTRUCTOR(...) ::std::has_virtual_destructor_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool hasVirtualDestructor = ZB_HAS_VIRTUAL_DESTRUCTOR(T);

} // namespace zb
