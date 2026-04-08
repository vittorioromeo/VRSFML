#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Internal helper for `SFML_BASE_TOKEN_PASTE`; do not use directly
///
////////////////////////////////////////////////////////////
#define SFML_BASE_TOKEN_PASTE_IMPL(x, y) x##y


////////////////////////////////////////////////////////////
/// \brief Concatenate two preprocessor tokens after macro expansion
///
/// Equivalent to `x##y`, but performs an extra round of macro
/// expansion on `x` and `y` first. Used to build unique identifiers
/// from `__LINE__` (e.g. inside `SFML_BASE_SCOPE_GUARD`).
///
////////////////////////////////////////////////////////////
#define SFML_BASE_TOKEN_PASTE(x, y) SFML_BASE_TOKEN_PASTE_IMPL(x, y)
