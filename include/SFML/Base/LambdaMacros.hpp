#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Apply `always_inline` and `flatten` attributes to a lambda
///
/// Can be placed between a lambda's capture list and parameter list
/// to force the compiler to always inline the lambda body and to
/// also inline every call inside it. Useful in hot dispatch loops.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN __attribute__((always_inline, flatten))


////////////////////////////////////////////////////////////
/// \brief Apply just the `always_inline` attribute to a lambda
///
////////////////////////////////////////////////////////////
#define SFML_BASE_LAMBDA_ALWAYS_INLINE __attribute__((always_inline))
