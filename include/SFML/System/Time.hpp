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
    /// \brief Default constructor
    ///
    /// Sets the time value to zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Time() = default;


    ////////////////////////////////////////////////////////////
    /// \brief Construct from microseconds
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Time(const base::I64 microseconds) :
        m_microseconds(microseconds)
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the time value as a number of seconds
    ///
    /// \return Time in seconds
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
    /// \return Time in milliseconds
    ///
    /// \see `asSeconds`, `asMicroseconds`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::I32 asMilliseconds() const
    {
        return static_cast<base::I32>(static_cast<float>(m_microseconds) / 1000.f);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the time value as a number of microseconds
    ///
    /// \return Time in microseconds
    ///
    /// \see `asSeconds`, `asMilliseconds`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::I64 asMicroseconds() const
    {
        return m_microseconds;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of `operator==` to compare two time values
    ///
    /// \param rhs Right operand (a time)
    ///
    /// \return `true` if both time values are equal
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const Time& rhs) const = default;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::I64 m_microseconds{}; //!< Time value stored as microseconds
};


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Construct a time value from a number of seconds
///
/// \param amount Number of seconds
///
/// \return Time value constructed from the amount of seconds
///
/// \see `milliseconds`, `microseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time seconds(const float amount)
{
    return Time(static_cast<base::I64>(amount * 1'000'000.f));
}

////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Construct a time value from a number of milliseconds
///
/// \param amount Number of milliseconds
///
/// \return Time value constructed from the amount of milliseconds
///
/// \see `seconds`, `microseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time milliseconds(const base::I32 amount)
{
    return Time(amount * 1000);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Construct a time value from a number of microseconds
///
/// \param amount Number of microseconds
///
/// \return Time value constructed from the amount of microseconds
///
/// \see `seconds`, `milliseconds`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time microseconds(const base::I64 amount)
{
    return Time(amount);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of `operator<` to compare two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `true` if `lhs` is lesser than `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator<(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() < rhs.asMicroseconds();
}

////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of `operator>` to compare two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `true` if `lhs` is greater than `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator>(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() > rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of `operator<=` to compare two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `true` if `lhs` is lesser or equal than `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator<=(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() <= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of `operator>=` to compare two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `true` if `lhs` is greater or equal than `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator>=(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() >= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of unary `operator-` to negate a time value
///
/// \param rhs Right operand (a time)
///
/// \return Opposite of the time value
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator-(const Time rhs)
{
    return microseconds(-rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator+` to add two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return Sum of the two times values
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator+(const Time lhs, const Time rhs)
{
    return microseconds(lhs.asMicroseconds() + rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator+=` to add/assign two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return Sum of the two times values
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator+=(Time& lhs, const Time rhs)
{
    return lhs = lhs + rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator-` to subtract two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return Difference of the two times values
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator-(const Time lhs, const Time rhs)
{
    return microseconds(lhs.asMicroseconds() - rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator-=` to subtract/assign two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return Difference of the two times values
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator-=(Time& lhs, const Time rhs)
{
    return lhs = lhs - rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*` to scale a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator*(Time lhs, float rhs)
{
    return seconds(lhs.asSeconds() * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*` to scale a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator*(const Time lhs, const base::I64 rhs)
{
    return microseconds(lhs.asMicroseconds() * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*` to scale a time value
///
/// \param lhs  Left operand (a number)
/// \param rhs Right operand (a time)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator*(const float lhs, const Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*` to scale a time value
///
/// \param lhs  Left operand (a number)
/// \param rhs Right operand (a time)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator*(const base::I64 lhs, const Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*=` to scale/assign a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator*=(Time& lhs, const float rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator*=` to scale/assign a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` multiplied by `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator*=(Time& lhs, const base::I64 rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator/` to scale a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` divided by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator/(const Time lhs, const float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return seconds(lhs.asSeconds() / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator/` to scale a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` divided by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator/(const Time lhs, const base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return microseconds(lhs.asMicroseconds() / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator/=` to scale/assign a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` divided by `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator/=(Time& lhs, const float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator/=` to scale/assign a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a number)
///
/// \return `lhs` divided by `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator/=(Time& lhs, const base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator/` to compute the ratio of two time values
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `lhs` divided by `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr float operator/(const Time lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator/ cannot divide by 0");
    return lhs.asSeconds() / rhs.asSeconds();
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator%` to compute remainder of a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `lhs` modulo `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Time operator%(const Time lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator% cannot modulus by 0");
    return microseconds(lhs.asMicroseconds() % rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
/// \relates Time
/// \brief Overload of binary `operator%=` to compute/assign remainder of a time value
///
/// \param lhs  Left operand (a time)
/// \param rhs Right operand (a time)
///
/// \return `lhs` modulo `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Time& operator%=(Time& lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator%= cannot modulus by 0");
    return lhs = lhs % rhs;
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Time
/// \ingroup system
///
/// `sf::Time` encapsulates a time value in a flexible way.
/// It allows to define a time value either as a number of
/// seconds, milliseconds or microseconds. It also works the
/// other way round: you can read a time value as either
/// a number of seconds, milliseconds or microseconds. It
/// even interoperates with the `<chrono>` header. You can
/// construct an `sf::Time` from a `chrono::duration` and read
/// any `sf::Time` as a chrono::duration.
///
/// By using such a flexible interface, the API doesn't
/// impose any fixed type or resolution for time values,
/// and let the user choose its own favorite representation.
///
/// Time values support the usual mathematical operations:
/// you can add or subtract two times, multiply or divide
/// a time by a number, compare two times, etc.
///
/// Since they represent a time span and not an absolute time
/// value, times can also be negative.
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
///
/// sf::Time t4 = std::chrono::milliseconds(250);
/// std::chrono::microseconds micro2 = t4.toDuration(); // 250'000us
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
