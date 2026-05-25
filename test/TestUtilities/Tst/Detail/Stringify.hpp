#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Forward-declaration-only header. Carries the `StringMaker`
// template (primary + built-in declarations) plus the out-of-line
// stringification helpers; the actual `sf::base::String` returns are
// all defined in `Stringify.cpp` so this header NEVER pulls
// `<SFML/Base/String.hpp>` -- ~10ms saved per including TU.
////////////////////////////////////////////////////////////


#include "Tst/TstFwd.hpp" // IWYU pragma: export


namespace tst
{
////////////////////////////////////////////////////////////
/// \brief User-overridable stringifier for arbitrary types.
///
/// The primary template intentionally has no body: every type the
/// runtime needs to stringify (failure decomposition, `INFO`/`CAPTURE`
/// chain values) must either name a built-in specialization below or
/// be supplied by a user-side specialization (see the various
/// `Stringify*Util.hpp` shims).
///
/// For unspecialized types `tst::stringify` falls back to a generic
/// placeholder -- see the `if constexpr` dispatch in
/// `Tst/Detail/Decompose.hpp`.
////////////////////////////////////////////////////////////
template <typename T>
struct StringMaker;


namespace detail
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::String defaultStringification();

////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::String stringifyBool(bool v);
[[nodiscard]] sf::base::String stringifyChar(char v);
[[nodiscard]] sf::base::String stringifySignedLongLong(long long v);
[[nodiscard]] sf::base::String stringifyUnsignedLongLong(unsigned long long v);
[[nodiscard]] sf::base::String stringifyDouble(double v);
[[nodiscard]] sf::base::String stringifyCString(const char* v);
[[nodiscard]] sf::base::String stringifyStringView(sf::base::StringView v);
[[nodiscard]] sf::base::String stringifyPtr(const void* v);

} // namespace detail


////////////////////////////////////////////////////////////
// Built-in specializations. Bodies live out-of-line in `Stringify.cpp`
// so this header does not need `<SFML/Base/String.hpp>` to be parsed.
////////////////////////////////////////////////////////////
template <>
struct StringMaker<bool>
{
    static sf::base::String convert(bool v);
};

template <>
struct StringMaker<char>
{
    static sf::base::String convert(char v);
};

template <>
struct StringMaker<signed char>
{
    static sf::base::String convert(signed char v);
};

template <>
struct StringMaker<unsigned char>
{
    static sf::base::String convert(unsigned char v);
};

template <>
struct StringMaker<short>
{
    static sf::base::String convert(short v);
};

template <>
struct StringMaker<unsigned short>
{
    static sf::base::String convert(unsigned short v);
};

template <>
struct StringMaker<int>
{
    static sf::base::String convert(int v);
};

template <>
struct StringMaker<unsigned int>
{
    static sf::base::String convert(unsigned int v);
};

template <>
struct StringMaker<long>
{
    static sf::base::String convert(long v);
};

template <>
struct StringMaker<unsigned long>
{
    static sf::base::String convert(unsigned long v);
};

template <>
struct StringMaker<long long>
{
    static sf::base::String convert(long long v);
};

template <>
struct StringMaker<unsigned long long>
{
    static sf::base::String convert(unsigned long long v);
};

template <>
struct StringMaker<float>
{
    static sf::base::String convert(float v);
};

template <>
struct StringMaker<double>
{
    static sf::base::String convert(double v);
};

template <>
struct StringMaker<const char*>
{
    static sf::base::String convert(const char* v);
};

template <>
struct StringMaker<char*>
{
    static sf::base::String convert(char* v);
};

} // namespace tst
