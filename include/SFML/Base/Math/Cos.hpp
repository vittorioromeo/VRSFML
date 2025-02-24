#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(cos)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(cos)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
