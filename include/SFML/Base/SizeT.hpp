#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `std::size_t` equivalent obtained without including `<cstddef>`
///
////////////////////////////////////////////////////////////
using SizeT = decltype(sizeof(int));

} // namespace sf::base
