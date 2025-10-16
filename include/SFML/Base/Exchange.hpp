#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Exchange the value of an object with a new value
///
/// Assigns `newVal` to `obj` and returns the old value of `obj`.
/// Equivalent to `std::exchange`.
///
/// \param obj    Object whose value to exchange
/// \param newVal New value to assign to `obj`
///
/// \return The old value of `obj`
///
////////////////////////////////////////////////////////////
template <typename T, typename U = T>
[[nodiscard, gnu::always_inline]] inline constexpr T exchange(T& obj, U&& newVal)
{
    T oldVal = static_cast<T&&>(obj);    // (move)
    obj      = static_cast<U&&>(newVal); // (forward)
    return oldVal;
}

} // namespace sf::base
