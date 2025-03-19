#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__make_unsigned)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MAKE_UNSIGNED(...) __make_unsigned(__VA_ARGS__)

#else

namespace sf::base
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

} // namespace sf::base

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MAKE_UNSIGNED(...) typename ::sf::base::MakeUnsignedImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using MakeUnsigned = SFML_BASE_MAKE_UNSIGNED(T);

} // namespace sf::base
