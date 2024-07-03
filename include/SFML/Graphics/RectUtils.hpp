////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/Graphics/Rect.hpp>

#include <optional>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Check the intersection between two rectangles
///
/// \param rectangle Rectangle to test
///
/// \return Intersection rectangle if intersecting, std::nullopt otherwise
///
/// \see contains
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] SFML_GRAPHICS_API constexpr std::optional<Rect<T>> findIntersection(const Rect<T>& rect0, const Rect<T>& rect1);

} // namespace sf


#include <SFML/Graphics/RectUtils.inl>


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////

extern template class sf::Rect<float>;
extern template class sf::Rect<double>;
extern template class sf::Rect<long double>;
extern template class sf::Rect<int>;
extern template class sf::Rect<unsigned int>;


////////////////////////////////////////////////////////////
/// \class sf::Rect
/// \ingroup graphics
///
/// A rectangle is defined by its top-left corner and its size.
/// It is a very simple class defined for convenience, so
/// its member variables (position and size) are public
/// and can be accessed directly, just like the vector classes
/// (Vector2 and Vector3).
///
/// To keep things simple, sf::Rect doesn't define
/// functions to emulate the properties that are not directly
/// members (such as right, bottom, etc.), it rather
/// only provides intersection functions.
///
/// sf::Rect uses the usual rules for its boundaries:
/// \li The left and top edges are included in the rectangle's area
/// \li The right and bottom edges are excluded from the rectangle's area
///
/// This means that sf::IntRect({0, 0}, {1, 1}) and sf::IntRect({1, 1}, {1, 1})
/// don't intersect.
///
/// sf::Rect is a template and may be used with any numeric type, but
/// for simplicity type aliases for the instantiations used by SFML are given:
/// \li sf::Rect<int> is sf::IntRect
/// \li sf::Rect<float> is sf::FloatRect
///
/// So that you don't have to care about the template syntax.
///
/// Usage example:
/// \code
/// // Define a rectangle, located at (0, 0) with a size of 20x5
/// sf::IntRect r1({0, 0}, {20, 5});
///
/// // Define another rectangle, located at (4, 2) with a size of 18x10
/// sf::Vector2i position(4, 2);
/// sf::Vector2i size(18, 10);
/// sf::IntRect r2(position, size);
///
/// // Test intersections with the point (3, 1)
/// bool b1 = r1.contains({3, 1}); // true
/// bool b2 = r2.contains({3, 1}); // false
///
/// // Test the intersection between r1 and r2
/// std::optional<sf::IntRect> result = r1.findIntersection(r2);
/// // result.has_value() == true
/// // result.value() == sf::IntRect({4, 2}, {16, 3})
/// \endcode
///
////////////////////////////////////////////////////////////