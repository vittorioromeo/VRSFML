// Intentionally include multiple times.
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(a, b) __##a(b)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_CHECK_BUILTIN(x) SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(has_builtin, x)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_HAS_MATH_BUILTIN(name) \
    SFML_BASE_PRIV_CHECK_BUILTIN(name) && SFML_BASE_PRIV_CHECK_BUILTIN(name##f) && SFML_BASE_PRIV_CHECK_BUILTIN(name##l)
