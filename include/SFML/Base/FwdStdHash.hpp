#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#ifdef __CLANGD__

    #include <functional> // IWYU pragma: export

#else

    #if __has_include(<__config>)
        #include <__config>

_LIBCPP_BEGIN_NAMESPACE_STD

template <typename>
struct hash;

_LIBCPP_END_NAMESPACE_STD

    #else

namespace std
{

template <typename>
struct hash;

} // namespace std

    #endif

#endif
