#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `std::ptrdiff_t` equivalent obtained without including `<cstddef>`
///
////////////////////////////////////////////////////////////
using PtrDiffT = decltype(static_cast<int*>(nullptr) - static_cast<int*>(nullptr));

} // namespace sf::base
