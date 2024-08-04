#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>


////////////////////////////////////////////////////////////
/// \file
///
/// \brief File containing SFML_DEFINE_DISCRETE_GPU_PREFERENCE
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// \def SFML_DEFINE_DISCRETE_GPU_PREFERENCE
///
/// \brief A macro to encourage usage of the discrete GPU
///
/// In order to inform the Nvidia/AMD driver that an SFML
/// application could benefit from using the more powerful
/// discrete GPU, special symbols have to be publicly
/// exported from the final executable.
///
/// SFML defines a helper macro to easily do this.
///
/// Place SFML_DEFINE_DISCRETE_GPU_PREFERENCE in the
/// global scope of a source file that will be linked into
/// the final executable. Typically it is best to place it
/// where the main function is also defined.
///
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

#define SFML_DEFINE_DISCRETE_GPU_PREFERENCE                                                  \
    extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement                  = 1; \
    extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;

#else

#define SFML_DEFINE_DISCRETE_GPU_PREFERENCE

#endif
