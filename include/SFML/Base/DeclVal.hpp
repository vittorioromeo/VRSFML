#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `std::declval` replacement, for use in unevaluated contexts only
///
/// Returns an rvalue reference to a hypothetical `T` without requiring
/// `T` to be constructible. Calling this function at runtime is
/// undefined behavior -- it must only appear inside `decltype`,
/// `requires`, `noexcept`, or other unevaluated operands.
///
////////////////////////////////////////////////////////////
template <typename T>
T&& declVal() noexcept;

} // namespace sf::base
