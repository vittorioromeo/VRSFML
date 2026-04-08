#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Forward declarations from `<string>`
///
/// Pulls in the standard library's internal `string` forward header
/// (`<bits/stringfwd.h>` on libstdc++, `<__fwd/string.h>` on libc++)
/// when available, falling back to the full `<string>` header
/// otherwise. Use this when a public SFML header only needs to
/// reference `std::string` by pointer/reference.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#ifdef __CLANGD__

    #include <string> // IWYU pragma: export

#else

    #if __has_include(<bits/stringfwd.h>)

        #include <bits/stringfwd.h> // IWYU pragma: export

    #elif __has_include(<__fwd/string.h>)

        #include <__fwd/string.h> // IWYU pragma: export

    #else

        #include <string> // IWYU pragma: export

    #endif

#endif
