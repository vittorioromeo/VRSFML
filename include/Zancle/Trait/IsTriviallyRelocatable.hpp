#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Trait/IsTriviallyCopyable.hpp"


namespace za::priv
{
#if __has_builtin(__builtin_is_cpp_trivially_relocatable)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) __builtin_is_cpp_trivially_relocatable(__VA_ARGS__)

#elif __has_builtin(__is_trivially_relocatable)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) __is_trivially_relocatable(__VA_ARGS__)

#else

    ////////////////////////////////////////////////////////////
    #define ZA_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) false

#endif


////////////////////////////////////////////////////////////
template <typename T>
inline consteval bool hasEnabledTrivialRelocation()
{
    if constexpr (requires { T::enableTrivialRelocation; })
    {
        return T::enableTrivialRelocation;
    }
    else
    {
        return false;
    }
}

} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool enableTrivialRelocation = priv::hasEnabledTrivialRelocation<T>();

} // namespace za


////////////////////////////////////////////////////////////
#define ZA_IS_TRIVIALLY_RELOCATABLE(...)                                                             \
    (ZA_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(__VA_ARGS__) || ZA_IS_TRIVIALLY_COPYABLE(__VA_ARGS__) || \
     ::za::enableTrivialRelocation<__VA_ARGS__>)


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyRelocatable = ZA_IS_TRIVIALLY_RELOCATABLE(T);

} // namespace za


////////////////////////////////////////////////////////////
/// \file
///
/// \brief Detection of trivially relocatable types
///
/// Trivial relocation is the property that a type's value can be
/// transferred between two storage locations by `memcpy` followed by
/// not running its destructor on the source. Zancle containers
/// (`Vector`, `SmallVector`, `InPlaceVector`, etc.) use this property
/// to skip per-element move-and-destroy loops on grow/relocate.
///
/// A type `T` is considered trivially relocatable when any of the
/// following holds:
/// - the compiler reports it as such via a builtin
///   (`__builtin_is_cpp_trivially_relocatable` or
///   `__is_trivially_relocatable`)
/// - it is trivially copyable
/// - it opts in by declaring an `enum : bool { enableTrivialRelocation = true }`
///
/// User code can opt in by adding the `enableTrivialRelocation` enum
/// to a class, e.g. when the class manages a heap buffer in a way
/// that survives a `memcpy` (`Vector` itself is the canonical example).
///
////////////////////////////////////////////////////////////
