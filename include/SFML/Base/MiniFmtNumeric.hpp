#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/MiniFmt.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
    requires(isIntegral<T> || isFloatingPoint<T>)
inline constexpr char formatArgDefaultAlign<T> = '>';


////////////////////////////////////////////////////////////
/// \brief Integer `formatArg`. Body defined in `MiniFmtNumeric.cpp`;
/// explicit instantiations cover every standard integer type.
////////////////////////////////////////////////////////////
template <typename T>
    requires isIntegral<T>
SFML_SYSTEM_API void formatArg(FormatSink& sink, const T& arg, const FormatSpec& spec);


////////////////////////////////////////////////////////////
/// \brief Floating-point `formatArg`. Body defined in `MiniFmtNumeric.cpp`;
/// explicit instantiations cover `float` and `double`.
////////////////////////////////////////////////////////////
template <typename T>
    requires isFloatingPoint<T>
SFML_SYSTEM_API void formatArg(FormatSink& sink, const T& arg, const FormatSpec& spec);


////////////////////////////////////////////////////////////
// `extern template` declarations: tell consuming TUs not to implicitly
// instantiate -- the explicit instantiations in `MiniFmtNumeric.cpp`
// provide the symbols. Custom integer/float types not listed here would
// fail to link via the template path -- they should provide their own
// ADL `formatArg` overload instead.
//
// `dispatchFormatArg<T>` and `erasedDispatch<T>` are also pre-instantiated
// here, since they always appear in lockstep with `formatArg<T>` along the
// type-erased dispatch path. Both are called either through a function
// pointer (`erasedDispatch`) or directly by it (`dispatchFormatArg`), so
// the loss of inlining at consumer call sites is negligible.
////////////////////////////////////////////////////////////
#define SFML_BASE_MINIFMT_EXTERN(T)                                                                            \
    extern template SFML_SYSTEM_API void formatArg<T>(FormatSink&, const T&, const FormatSpec&);               \
    extern template SFML_SYSTEM_API void priv::dispatchFormatArg<T>(FormatSink&, const T&, const FormatSpec&); \
    extern template SFML_SYSTEM_API void priv::erasedDispatch<T>(FormatSink&, const void*, const FormatSpec&)

SFML_BASE_MINIFMT_EXTERN(bool);
SFML_BASE_MINIFMT_EXTERN(char);
SFML_BASE_MINIFMT_EXTERN(signed char);
SFML_BASE_MINIFMT_EXTERN(unsigned char);
SFML_BASE_MINIFMT_EXTERN(short);
SFML_BASE_MINIFMT_EXTERN(unsigned short);
SFML_BASE_MINIFMT_EXTERN(int);
SFML_BASE_MINIFMT_EXTERN(unsigned int);
SFML_BASE_MINIFMT_EXTERN(long);
SFML_BASE_MINIFMT_EXTERN(unsigned long);
SFML_BASE_MINIFMT_EXTERN(long long);
SFML_BASE_MINIFMT_EXTERN(unsigned long long);
SFML_BASE_MINIFMT_EXTERN(float);
SFML_BASE_MINIFMT_EXTERN(double);

#undef SFML_BASE_MINIFMT_EXTERN


////////////////////////////////////////////////////////////
/// \brief Enum `formatArg`: dispatches to the integral overload of the
/// underlying type. Inline because it does no real work beyond a cast.
////////////////////////////////////////////////////////////
template <typename T>
    requires isEnum<T>
[[gnu::always_inline]] inline void formatArg(FormatSink& sink, const T& arg, const FormatSpec& spec)
{
    formatArg(sink, static_cast<UnderlyingType<T>>(arg), spec);
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Opt-in extension: numeric `formatArg` overloads (integers,
/// floats, enums).
///
/// Bodies live out-of-line in `MiniFmtNumeric.cpp` and are explicitly
/// instantiated for every standard arithmetic type. The header therefore
/// does **not** pull in `<SFML/Base/ToChars.hpp>` (or its transitive
/// builtins / math / trait dependencies); call sites only see the
/// declarations + the lightweight `IsIntegral` / `IsFloatingPoint` /
/// `IsEnum` / `UnderlyingType` traits that the constraints need.
///
/// Transitively re-exported by `<SFML/Base/MiniFmtFormat.hpp>` and
/// `<SFML/System/Err.hpp>`, so most callers get it for free.
////////////////////////////////////////////////////////////
