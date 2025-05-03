#pragma once

#include "SFML/Base/Array.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#define EXACT_ARRAY(type, name, requiredCount, ...) \
    type name[requiredCount] __VA_ARGS__;           \
    static_assert(::std::initializer_list<type> __VA_ARGS__.size() == requiredCount)
