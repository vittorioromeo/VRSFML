// Header for Zancle unit tests.
//
// For a new window module test case, include this header.
// Declares the `stringifyValue` ADL overload so the bespoke testing
// library can render `za::VideoMode` for failure output.

#pragma once

#include "ZancleBase/SizeT.hpp"


#ifdef ZA_RUN_DISPLAY_TESTS
inline constexpr bool skipDisplayTests = false;
#else
inline constexpr bool skipDisplayTests = true;
#endif


namespace za
{
struct VideoMode;

// Found by ADL when a `VideoMode` operand needs rendering for a failed assertion.
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const VideoMode& videoMode) noexcept;
} // namespace za
