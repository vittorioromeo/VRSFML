// Intentionally include multiple times.
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Trailer for the per-function math headers, undoes the helper macros
///
/// Each `SFML/Base/Math/<name>.hpp` ends by including this file so
/// that the macros defined by `Impl.hpp` and one of the wrapper
/// implementations (`ImplBuiltinWrapper.hpp` or `ImplStdForwarder.hpp`)
/// do not leak into other translation units that include the same
/// header in a different order.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#undef SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG
#undef SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG
#undef SFML_BASE_PRIV_HAS_MATH_BUILTIN
#undef SFML_BASE_PRIV_CHECK_BUILTIN
#undef SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN
