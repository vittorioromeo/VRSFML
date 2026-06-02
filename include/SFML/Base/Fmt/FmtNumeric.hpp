#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
    requires(isIntegral<T> || isFloatingPoint<T>)
inline constexpr char fmtArgDefaultAlign<T> = '>';


////////////////////////////////////////////////////////////
// `bool` formats as the string `"true"` / `"false"`, so left-align by
// default (matching string-like types). Overrides the numeric '>' above.
////////////////////////////////////////////////////////////
template <>
inline constexpr char fmtArgDefaultAlign<bool> = '<';


////////////////////////////////////////////////////////////
// `char` formats as a glyph by default (matching `libfmt` / `std::format`),
// so left-align like other string-like types. Overrides the numeric '>' above.
// `signed char` / `unsigned char` keep numeric right-alignment.
////////////////////////////////////////////////////////////
template <>
inline constexpr char fmtArgDefaultAlign<char> = '<';


////////////////////////////////////////////////////////////
/// \brief Integer `fmtArg`. Body defined in `FmtNumeric.cpp`;
/// explicit instantiations cover every standard integer type.
////////////////////////////////////////////////////////////
template <typename T>
    requires isIntegral<T>
[[nodiscard]] SFML_SYSTEM_API FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec);


////////////////////////////////////////////////////////////
/// \brief `char` `fmtArg`: formats as a single-byte glyph by default and
/// on `:c`, dispatches to the integer overload on `:d` / `:x` / `:X` /
/// `:o` / `:b`. Matches `libfmt` / `std::format` semantics; `signed char`
/// and `unsigned char` still go through the integer template above.
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API FmtResult fmtArg(FmtSink& sink, const char& arg, const FmtSpec& spec);


////////////////////////////////////////////////////////////
/// \brief Floating-point `fmtArg`. Body defined in `FmtNumeric.cpp`;
/// explicit instantiations cover every standard floating-point type.
////////////////////////////////////////////////////////////
template <typename T>
    requires isFloatingPoint<T>
[[nodiscard]] SFML_SYSTEM_API FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec);


////////////////////////////////////////////////////////////
// `extern template` declarations: tell consuming TUs not to implicitly
// instantiate -- the explicit instantiations in `FmtNumeric.cpp`
// provide the symbols. Custom integer/float types not listed here would
// fail to link via the template path -- they should provide their own
// ADL `fmtArg` overload instead.
//
// `dispatchFmtArg<T>` and `dispatchFmtArgErased<T>` are also pre-instantiated
// here, since they always appear in lockstep with `fmtArg<T>` along the
// type-erased dispatch path. Both are called either through a function
// pointer (`dispatchFmtArgErased`) or directly by it (`dispatchFmtArg`), so
// the loss of inlining at consumer call sites is negligible.
////////////////////////////////////////////////////////////
#define SFML_BASE_FMT_EXTERN(T)                                                            \
    extern template FmtResult fmtArg<T>(FmtSink&, const T&, const FmtSpec&);               \
    extern template FmtResult priv::dispatchFmtArg<T>(FmtSink&, const T&, const FmtSpec&); \
    extern template FmtResult priv::dispatchFmtArgErased<T>(FmtSink&, const void*, const FmtSpec&)

SFML_BASE_FMT_EXTERN(bool);

SFML_BASE_FMT_EXTERN(signed char);
SFML_BASE_FMT_EXTERN(short);
SFML_BASE_FMT_EXTERN(int);
SFML_BASE_FMT_EXTERN(long);
SFML_BASE_FMT_EXTERN(long long);

SFML_BASE_FMT_EXTERN(unsigned char);
SFML_BASE_FMT_EXTERN(unsigned short);
SFML_BASE_FMT_EXTERN(unsigned int);
SFML_BASE_FMT_EXTERN(unsigned long);
SFML_BASE_FMT_EXTERN(unsigned long long);

SFML_BASE_FMT_EXTERN(float);
SFML_BASE_FMT_EXTERN(double);
SFML_BASE_FMT_EXTERN(long double);

#undef SFML_BASE_FMT_EXTERN

// `char` has a non-template `fmtArg` (declared above), so only the
// dispatcher instantiations are pre-emitted here.
extern template FmtResult priv::dispatchFmtArg<char>(FmtSink&, const char&, const FmtSpec&);
extern template FmtResult priv::dispatchFmtArgErased<char>(FmtSink&, const void*, const FmtSpec&);


////////////////////////////////////////////////////////////
/// \brief Enum `fmtArg`: dispatches to the integral overload of the
/// underlying type. Inline because it does no real work beyond a cast.
////////////////////////////////////////////////////////////
template <typename T>
    requires isEnum<T>
[[nodiscard, gnu::always_inline]] inline FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec)
{
    return fmtArg(sink, static_cast<UnderlyingType<T>>(arg), spec);
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Opt-in extension: numeric `fmtArg` overloads (integers,
/// floats, enums).
///
/// Bodies live out-of-line in `FmtNumeric.cpp` and are explicitly
/// instantiated for every standard arithmetic type. The header therefore
/// does **not** pull in `<SFML/Base/ToChars.hpp>` (or its transitive
/// builtins / math / trait dependencies); call sites only see the
/// declarations + the lightweight `IsIntegral` / `IsFloatingPoint` /
/// `IsEnum` / `UnderlyingType` traits that the constraints need.
///
/// Transitively re-exported by `<SFML/Base/Fmt/FmtToString.hpp>` and
/// `<SFML/System/Err.hpp>`, so most callers get it for free.
////////////////////////////////////////////////////////////
