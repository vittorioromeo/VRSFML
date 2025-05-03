#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#ifndef SFML_ENABLE_PCH
    #error "PCH source included, but `SFML_ENABLE_PCH` was not defined"
#else // SFML_ENABLE_PCH

    #ifdef SFML_BUILD_TEST_SUITE

        #define DOCTEST_CONFIG_IMPLEMENT 1
        #include <doctest/doctest.h>

    #endif // SFML_BUILD_TEST_SUITE

#endif // SFML_ENABLE_PCH
