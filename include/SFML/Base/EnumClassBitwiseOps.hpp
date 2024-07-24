#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/Traits/UnderlyingType.hpp>


////////////////////////////////////////////////////////////
#define SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(enumType)                                                        \
                                                                                                                 \
    [[nodiscard, gnu::always_inline, gnu::const]] inline bool operator!(enumType lhs) noexcept                   \
    {                                                                                                            \
        return !static_cast<bool>(lhs);                                                                          \
    }                                                                                                            \
                                                                                                                 \
    [[nodiscard, gnu::always_inline, gnu::const]] inline enumType operator|(enumType lhs, enumType rhs) noexcept \
    {                                                                                                            \
        return enumType{static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) |                                  \
                        static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs)};                                  \
    }                                                                                                            \
                                                                                                                 \
    [[nodiscard, gnu::always_inline, gnu::const]] inline enumType operator&(enumType lhs, enumType rhs) noexcept \
    {                                                                                                            \
        return enumType{static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) &                                  \
                        static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs)};                                  \
    }                                                                                                            \
                                                                                                                 \
    [[nodiscard, gnu::always_inline, gnu::const]] inline enumType operator^(enumType lhs, enumType rhs) noexcept \
    {                                                                                                            \
        return enumType{static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs) ^                                  \
                        static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(rhs)};                                  \
    }                                                                                                            \
                                                                                                                 \
    [[nodiscard, gnu::always_inline, gnu::const]] inline enumType operator~(enumType lhs) noexcept               \
    {                                                                                                            \
        return enumType{~static_cast<SFML_BASE_UNDERLYING_TYPE(enumType)>(lhs)};                                 \
    }                                                                                                            \
                                                                                                                 \
    [[gnu::always_inline]] inline enumType& operator|=(enumType& lhs, enumType rhs) noexcept                     \
    {                                                                                                            \
        return lhs = (lhs | rhs);                                                                                \
    }                                                                                                            \
                                                                                                                 \
    [[gnu::always_inline]] inline enumType& operator&=(enumType& lhs, enumType rhs) noexcept                     \
    {                                                                                                            \
        return lhs = (lhs & rhs);                                                                                \
    }                                                                                                            \
                                                                                                                 \
    using sfPrivSwallowSemicolonBitwiseOps = void
