// Intentionally include multiple times.
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Internal helpers for `sf::base::math::*` math wrappers
///
/// Provides `SFML_BASE_PRIV_HAS_MATH_BUILTIN(name)`, which checks
/// whether all three of `__builtin_<name>`, `__builtin_<name>f`, and
/// `__builtin_<name>l` are available. Used to decide whether the
/// per-function math headers can call compiler builtins (preferred,
/// no `<cmath>` include) or must fall back to `std::<name>` from
/// `<cmath>`.
///
/// Intentionally not pragma-once: each math header includes this in
/// its own translation context to define a fresh set of macros.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(a, b) __##a(b)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_CHECK_BUILTIN(x) SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(has_builtin, x)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_HAS_MATH_BUILTIN(name) \
    SFML_BASE_PRIV_CHECK_BUILTIN(name) && SFML_BASE_PRIV_CHECK_BUILTIN(name##f) && SFML_BASE_PRIV_CHECK_BUILTIN(name##l)
