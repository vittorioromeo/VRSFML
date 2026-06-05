#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__make_unsigned)

    ////////////////////////////////////////////////////////////
    #define ZA_MAKE_UNSIGNED(...) __make_unsigned(__VA_ARGS__)

#else

namespace za::priv
{
////////////////////////////////////////////////////////////
template <typename T>
struct MakeUnsignedImpl;


////////////////////////////////////////////////////////////
// clang-format off
template <> struct MakeUnsignedImpl<char8_t>            { using type = char8_t; };
template <> struct MakeUnsignedImpl<char16_t>           { using type = char16_t; };
template <> struct MakeUnsignedImpl<char32_t>           { using type = char32_t; };
template <> struct MakeUnsignedImpl<         char>      { using type = unsigned char; };
template <> struct MakeUnsignedImpl<  signed char>      { using type = unsigned char; };
template <> struct MakeUnsignedImpl<unsigned char>      { using type = unsigned char; };
template <> struct MakeUnsignedImpl<  signed short>     { using type = unsigned short; };
template <> struct MakeUnsignedImpl<unsigned short>     { using type = unsigned short; };
template <> struct MakeUnsignedImpl<  signed int>       { using type = unsigned int; };
template <> struct MakeUnsignedImpl<unsigned int>       { using type = unsigned int; };
template <> struct MakeUnsignedImpl<  signed long>      { using type = unsigned long; };
template <> struct MakeUnsignedImpl<unsigned long>      { using type = unsigned long; };
template <> struct MakeUnsignedImpl<  signed long long> { using type = unsigned long long; };
template <> struct MakeUnsignedImpl<unsigned long long> { using type = unsigned long long; };
// clang-format on

} // namespace za::priv

    ////////////////////////////////////////////////////////////
    #define ZA_MAKE_UNSIGNED(...) typename ::za::priv::MakeUnsignedImpl<__VA_ARGS__>::type

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
using MakeUnsigned = ZA_MAKE_UNSIGNED(T);

} // namespace za
