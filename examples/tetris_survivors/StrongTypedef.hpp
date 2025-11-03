#pragma once


////////////////////////////////////////////////////////////
#define TSURV_DEFINE_STRONG_TYPEDEF(name, underlyingType)                                                             \
                                                                                                                      \
    class [[nodiscard]] name final                                                                                    \
    {                                                                                                                 \
    private:                                                                                                          \
        underlyingType m_value;                                                                                       \
                                                                                                                      \
    public:                                                                                                           \
        using UnderlyingType = underlyingType;                                                                        \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr static name from(const auto& value) noexcept           \
        {                                                                                                             \
            return name{static_cast<underlyingType>(value)};                                                          \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline]] constexpr name() noexcept = default;                                        \
                                                                                                                      \
        [[nodiscard, gnu::always_inline]] constexpr explicit name(const underlyingType value) noexcept :              \
            m_value{value}                                                                                            \
        {                                                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[gnu::always_inline]] constexpr name& operator=(const underlyingType rhs) noexcept                           \
        {                                                                                                             \
            m_value = rhs;                                                                                            \
            return *this;                                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr operator const underlyingType&() const noexcept        \
        {                                                                                                             \
            return m_value;                                                                                           \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr operator underlyingType&() noexcept                    \
        {                                                                                                             \
            return m_value;                                                                                           \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr underlyingType& toUnderlying() noexcept                \
        {                                                                                                             \
            return m_value;                                                                                           \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const underlyingType& toUnderlying() const noexcept    \
        {                                                                                                             \
            return m_value;                                                                                           \
        }                                                                                                             \
                                                                                                                      \
        [[gnu::always_inline]] constexpr name& operator++() noexcept                                                  \
        {                                                                                                             \
            ++m_value;                                                                                                \
            return *this;                                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline]] constexpr name operator++(int) noexcept                                     \
        {                                                                                                             \
            name temp = *this;                                                                                        \
            ++m_value;                                                                                                \
            return temp;                                                                                              \
        }                                                                                                             \
                                                                                                                      \
        [[gnu::always_inline]] constexpr name& operator--() noexcept                                                  \
        {                                                                                                             \
            --m_value;                                                                                                \
            return *this;                                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline]] constexpr name operator--(int) noexcept                                     \
        {                                                                                                             \
            name temp = *this;                                                                                        \
            --m_value;                                                                                                \
            return temp;                                                                                              \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const name&) const noexcept = default; \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr auto operator<(const name& rhs) const noexcept         \
        {                                                                                                             \
            return m_value < rhs.m_value;                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr auto operator<=(const name& rhs) const noexcept        \
        {                                                                                                             \
            return m_value <= rhs.m_value;                                                                            \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr auto operator>(const name& rhs) const noexcept         \
        {                                                                                                             \
            return m_value > rhs.m_value;                                                                             \
        }                                                                                                             \
                                                                                                                      \
        [[nodiscard, gnu::always_inline, gnu::pure]] constexpr auto operator>=(const name& rhs) const noexcept        \
        {                                                                                                             \
            return m_value >= rhs.m_value;                                                                            \
        }                                                                                                             \
    }


////////////////////////////////////////////////////////////
#ifdef _LIBCPP_BEGIN_NAMESPACE_STD

_LIBCPP_BEGIN_NAMESPACE_STD
////////////////////////////////////////////////////////////
template <typename>
struct hash;

////////////////////////////////////////////////////////////
template <typename T>
    requires requires { typename T::UnderlyingType; }
struct hash<T>
{
    [[nodiscard, gnu::always_inline, gnu::pure]] auto operator()(const T& value) const noexcept
    {
        return std::hash<typename T::UnderlyingType>()(value.toUnderlying());
    }
};
_LIBCPP_END_NAMESPACE_STD

#else

namespace std
{
////////////////////////////////////////////////////////////
template <typename>
struct hash;

////////////////////////////////////////////////////////////
template <typename T>
    requires requires { typename T::UnderlyingType; }
struct hash<T>
{
    [[nodiscard, gnu::always_inline, gnu::pure]] auto operator()(const T& value) const noexcept
    {
        return std::hash<typename T::UnderlyingType>()(value.toUnderlying());
    }
};

} // namespace std

#endif
