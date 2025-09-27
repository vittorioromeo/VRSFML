#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#ifdef __clang__

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_TRIVIAL_ABI [[clang::trivial_abi]]

#else

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_TRIVIAL_ABI

#endif
