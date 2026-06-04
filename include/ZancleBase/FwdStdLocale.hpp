#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Forward declarations from `<locale>`
///
/// Includes the standard library's internal `locale` forward header
/// (`<bits/localefwd.h>` on libstdc++, `<__fwd/locale.h>` on libc++)
/// when available, falling back to the full `<locale>` header
/// otherwise. This dramatically reduces compile time for headers that
/// only need to declare functions taking `std::locale&`.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#if __has_include(<bits/localefwd.h>)

    #include <bits/localefwd.h>

#elif __has_include(<__fwd/locale.h>)

    #include <__fwd/locale.h>

#else

    #include <locale>

#endif
