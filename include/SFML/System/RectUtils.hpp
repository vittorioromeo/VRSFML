#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Check the intersection between two rectangles
///
/// \param rect0 First rectangle
/// \param rect1 Second rectangle
///
/// \return Intersection rectangle if intersecting, `base::nullOpt` otherwise
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::pure]] SFML_SYSTEM_API base::Optional<Rect<T>> findIntersection(const Rect<T>& rect0, const Rect<T>& rect1);

} // namespace sf

#include "SFML/System/RectUtils.inl"


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
/// sf::base::Optional<sf::IntRect> result = r1.findIntersection(r2);
/// // result.hasValue() == true
/// // result.value() == sf::IntRect({4, 2}, {16, 3})
/// \endcode
///
////////////////////////////////////////////////////////////
