// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

#ifndef SFML_ENABLE_PCH
    #error "PCH source included, but `SFML_ENABLE_PCH` was not defined"
#else // SFML_ENABLE_PCH

    #ifdef SFML_BUILD_TEST_SUITE

        #undef DOCTEST_CONFIG_USE_STD_HEADERS
        #undef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

        #define DOCTEST_CONFIG_IMPLEMENT 1
        #include <doctest/doctest.h>

    #endif // SFML_BUILD_TEST_SUITE

#endif // SFML_ENABLE_PCH
