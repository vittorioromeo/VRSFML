#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Apply Clang's `[[clang::trivial_abi]]` attribute when supported
///
/// `trivial_abi` lets a non-trivial type be passed in registers (and
/// memcpy'd in calling conventions) by ignoring its non-trivial
/// special members at the ABI boundary. Used by `UniquePtr` so it can
/// be passed as cheaply as a raw pointer. Expands to nothing on
/// non-Clang compilers.
///
////////////////////////////////////////////////////////////
#ifdef __clang__

    #define SFML_BASE_TRIVIAL_ABI [[clang::trivial_abi]]

#else

    #define SFML_BASE_TRIVIAL_ABI

#endif
