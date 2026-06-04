#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Lightweight `<chrono>` include
///
/// Pulls in just the duration/clock declarations from libstdc++'s
/// internal `<bits/chrono.h>` when possible (much smaller than the
/// public `<chrono>` header), and falls back to the full header on
/// other libraries.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#ifdef __CLANGD__
    #include <chrono> // IWYU pragma: export
#endif


////////////////////////////////////////////////////////////
#if __has_include(<bits/chrono.h>)
    #include <bits/chrono.h> // IWYU pragma: export
#else
    #include <chrono> // IWYU pragma: export
#endif
