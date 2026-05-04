#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Lightweight `<thread>` include
///
/// Pulls in just the thread declarations from libstdc++'s
/// internal `<bits/std_thread.h>` when possible (much smaller than the
/// public `<thread>` header), and falls back to the full header on
/// other libraries.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#ifdef __CLANGD__
    #include <thread> // IWYU pragma: export
#endif


////////////////////////////////////////////////////////////
#if __has_include(<bits/std_thread.h>) && __has_include(<bits/this_thread_sleep.h>)
    #include <bits/std_thread.h>        // IWYU pragma: export
    #include <bits/this_thread_sleep.h> // IWYU pragma: export
#else
    #include <thread> // IWYU pragma: export
#endif
