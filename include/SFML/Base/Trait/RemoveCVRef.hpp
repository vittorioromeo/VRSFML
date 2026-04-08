#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief `std::remove_cvref` replacement
///
/// `RemoveCVRef<T>` strips both references and top-level cv-qualifiers
/// from `T`. The macro form `SFML_BASE_REMOVE_CVREF(T)` prefers the
/// compiler builtin when available; the type alias forwards to it.
///
/// `RemoveCVRefIndirect<T>` is the same trait expressed without the
/// builtin, which is required when used inside `requires` constraints
/// (where compiler builtins are sometimes not allowed).
///
////////////////////////////////////////////////////////////


#if __has_builtin(__remove_cvref)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CVREF(...) __remove_cvref(__VA_ARGS__)

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T>
struct RemoveCVRefImpl
{
    using type = SFML_BASE_REMOVE_CVREF(T);
};

} // namespace sf::base::priv

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveCVRefImpl                     { using type = T; };
template <typename T> struct RemoveCVRefImpl<T&>                 { using type = T; };
template <typename T> struct RemoveCVRefImpl<T&&>                { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T>            { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T&>           { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T&&>          { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T>         { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T&>        { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T&&>       { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T>   { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T&>  { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T&&> { using type = T; };
// clang-format on

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CVREF(...) typename ::sf::base::priv::RemoveCVRefImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveCVRef = SFML_BASE_REMOVE_CVREF(T);

////////////////////////////////////////////////////////////
template <typename T>
using RemoveCVRefIndirect = typename priv::RemoveCVRefImpl<T>::type; // `requires` constraints cannot use builtins

} // namespace sf::base
