#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Assign `newVal` to `obj` and return its old value
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
