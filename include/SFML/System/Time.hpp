#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Represents a time value
///
////////////////////////////////////////////////////////////
class Time
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a zero-valued time
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Time() = default;


    ////////////////////////////////////////////////////////////
    /// \brief Return the time value as a number of seconds
    ///
    /// \see `asMilliseconds`, `asMicroseconds`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr float asSeconds() const
    {
        return static_cast<float>(m_microseconds) / 1'000'000.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the time value as a number of milliseconds
    ///
    /// \see `asSeconds`, `asMicroseconds`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::I32 asMilliseconds() const
    {
        return static_cast<base::I32>(m_microseconds / 1000);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the time value as a number of microseconds
    ///
    /// \see `asSeconds`, `asMilliseconds`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::I64 asMicroseconds() const
    {
        return m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Equality comparison of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const Time& rhs) const = default;


    ////////////////////////////////////////////////////////////
    /// \brief Less-than comparison of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr bool operator<(const Time lhs, const Time rhs)
    {
        return lhs.m_microseconds < rhs.m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Greater-than comparison of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr bool operator>(const Time lhs, const Time rhs)
    {
        return lhs.m_microseconds > rhs.m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Less-than-or-equal comparison of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr bool operator<=(const Time lhs, const Time rhs)
    {
        return lhs.m_microseconds <= rhs.m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Greater-than-or-equal comparison of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr bool operator>=(const Time lhs, const Time rhs)
    {
        return lhs.m_microseconds >= rhs.m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Negation of a time
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator-(const Time rhs)
    {
        return Time(-rhs.m_microseconds);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Sum of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator+(const Time lhs, const Time rhs)
    {
        return Time(lhs.m_microseconds + rhs.m_microseconds);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound addition of two times
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator+=(Time& lhs, const Time rhs)
    {
        lhs.m_microseconds += rhs.m_microseconds;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Difference of two times
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator-(const Time lhs, const Time rhs)
    {
        return Time(lhs.m_microseconds - rhs.m_microseconds);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound subtraction of two times
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator-=(Time& lhs, const Time rhs)
    {
        lhs.m_microseconds -= rhs.m_microseconds;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Scale a time by a floating-point factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator*(const Time lhs, const float rhs)
    {
        return Time(static_cast<base::I64>(static_cast<float>(lhs.m_microseconds) * rhs));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Scale a time by an integer factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator*(const Time lhs, const base::I64 rhs)
    {
        return Time(lhs.m_microseconds * rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Scale a time by a floating-point factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator*(const float lhs, const Time rhs)
    {
        return rhs * lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Scale a time by an integer factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator*(const base::I64 lhs, const Time rhs)
    {
        return rhs * lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound scale of a time by a floating-point factor
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator*=(Time& lhs, const float rhs)
    {
        return lhs = lhs * rhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound scale of a time by an integer factor
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator*=(Time& lhs, const base::I64 rhs)
    {
        return lhs = lhs * rhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Divide a time by a floating-point divisor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator/(const Time lhs, const float rhs)
    {
        SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
        return Time(static_cast<base::I64>(static_cast<float>(lhs.m_microseconds) / rhs));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Divide a time by an integer divisor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator/(const Time lhs, const base::I64 rhs)
    {
        SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
        return Time(lhs.m_microseconds / rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound divide of a time by a floating-point divisor
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator/=(Time& lhs, const float rhs)
    {
        SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
        return lhs = lhs / rhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound divide of a time by an integer divisor
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator/=(Time& lhs, const base::I64 rhs)
    {
        SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
        return lhs = lhs / rhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Ratio of two times as a floating-point value
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr float operator/(const Time lhs, const Time rhs)
    {
        SFML_BASE_ASSERT(rhs.m_microseconds != 0 && "Time::operator/ cannot divide by 0");
        return static_cast<float>(lhs.m_microseconds) / static_cast<float>(rhs.m_microseconds);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Remainder of one time divided by another
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Time operator%(const Time lhs, const Time rhs)
    {
        SFML_BASE_ASSERT(rhs.m_microseconds != 0 && "Time::operator% cannot modulus by 0");
        return Time(lhs.m_microseconds % rhs.m_microseconds);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Compound remainder of one time divided by another
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend constexpr Time& operator%=(Time& lhs, const Time rhs)
    {
        SFML_BASE_ASSERT(rhs.m_microseconds != 0 && "Time::operator%= cannot modulus by 0");
        return lhs = lhs % rhs;
    }


    ////////////////////////////////////////////////////////////
    friend constexpr Time seconds(float amount);
    friend constexpr Time milliseconds(base::I32 amount);
    friend constexpr Time microseconds(base::I64 amount);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Construct from microseconds
    ///
    /// Prefer the `sf::microseconds`, `sf::milliseconds`, and
    /// `sf::seconds` factory functions for clarity at call sites.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Time(const base::I64 microseconds) :
        m_microseconds(microseconds)
    {
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::I64 m_microseconds{}; //!< Time value stored as microseconds
};


////////////////////////////////////////////////////////////
/// \brief Construct a time value from a number of seconds
///
/// \see `milliseconds`, `microseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time seconds(const float amount)
{
    return Time(static_cast<base::I64>(amount * 1'000'000.f));
}

////////////////////////////////////////////////////////////
/// \brief Construct a time value from a number of milliseconds
///
/// \see `seconds`, `microseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time milliseconds(const base::I32 amount)
{
    return Time(amount * 1000);
}


////////////////////////////////////////////////////////////
/// \brief Construct a time value from a number of microseconds
///
/// \see `seconds`, `milliseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time microseconds(const base::I64 amount)
{
    return Time(amount);
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Time
/// \ingroup system
///
/// `sf::Time` encapsulates a time value in a flexible way:
/// it can be defined or read as a number of seconds,
/// milliseconds, or microseconds, leaving the choice of
/// representation to the user.
///
/// `sf::Time` is intentionally decoupled from `<chrono>` to keep
/// this header lightweight; conversions between `sf::Time` and
/// `std::chrono::duration` are provided as opt-in helpers in
/// `SFML/System/TimeChronoUtil.hpp` (see `sf::TimeChronoUtil`).
///
/// Time values support the usual arithmetic and comparison
/// operations, and represent a span (so they may be negative).
///
/// Usage example:
/// \code
/// sf::Time t1 = sf::seconds(0.1f);
/// base::I32 milli = t1.asMilliseconds(); // 100
///
/// sf::Time t2 = sf::milliseconds(30);
/// base::I64 micro = t2.asMicroseconds(); // 30'000
///
/// sf::Time t3 = sf::microseconds(-800'000);
/// float sec = t3.asSeconds(); // -0.8
/// \endcode
///
/// \code
/// void update(sf::Time elapsed)
/// {
///    position += speed * elapsed.asSeconds();
/// }
///
/// update(sf::milliseconds(100));
/// \endcode
///
/// \see `sf::Clock`
///
////////////////////////////////////////////////////////////
