#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Intentionally suppress function template argument deduction
///
////////////////////////////////////////////////////////////
template <typename T>
struct NonDeduced
{
    using type = T;
};

} // namespace sf::base
