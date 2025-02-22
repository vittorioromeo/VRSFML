#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Traits/UnderlyingType.hpp"


////////////////////////////////////////////////////////////
#define SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(enumType)                                                                      \
                                                                                                                               \
    [[nodiscard, maybe_unused, gnu::always_inline, gnu::const]] inline bool operator!(enumType lhs) noexcept                   \
    {                                                                                                                          \
        return !static_cast<bool>(lhs);                                                                                        \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, maybe_unused, gnu::always_inline, gnu::const]] inline enumType operator|(enumType lhs, enumType rhs) noexcept \
    {                                                                                                                          \
        return static_cast<enumType>(static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) |                                   \
                                     static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs));                                   \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, maybe_unused, gnu::always_inline, gnu::const]] inline enumType operator&(enumType lhs, enumType rhs) noexcept \
    {                                                                                                                          \
        return static_cast<enumType>(static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) &                                   \
                                     static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs));                                   \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, maybe_unused, gnu::always_inline, gnu::const]] inline enumType operator^(enumType lhs, enumType rhs) noexcept \
    {                                                                                                                          \
        return static_cast<enumType>(static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) ^                                   \
                                     static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs));                                   \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, maybe_unused, gnu::always_inline, gnu::const]] inline enumType operator~(enumType lhs) noexcept               \
    {                                                                                                                          \
        return static_cast<enumType>(~static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs));                                  \
    }                                                                                                                          \
                                                                                                                               \
    [[maybe_unused, gnu::always_inline]] inline enumType& operator|=(enumType& lhs, enumType rhs) noexcept                     \
    {                                                                                                                          \
        return lhs = (lhs | rhs);                                                                                              \
    }                                                                                                                          \
                                                                                                                               \
    [[maybe_unused, gnu::always_inline]] inline enumType& operator&=(enumType& lhs, enumType rhs) noexcept                     \
    {                                                                                                                          \
        return lhs = (lhs & rhs);                                                                                              \
    }                                                                                                                          \
                                                                                                                               \
    using sfPrivSwallowSemicolonBitwiseOps = void
