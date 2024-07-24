#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


// Forward declare an Objective-C class
#ifdef __OBJC__
#define SFML_DECLARE_OBJC_CLASS(c) @class c
#else
#define SFML_DECLARE_OBJC_CLASS(c) typedef struct objc_object c
#endif
