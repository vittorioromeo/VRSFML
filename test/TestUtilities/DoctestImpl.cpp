// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

// Compile the doctest implementation in a single translation unit using the
// amalgamated single-header, instead of building the 33-file parts-based
// static library (doctest_with_main).

#undef DOCTEST_CONFIG_USE_STD_HEADERS
#undef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
