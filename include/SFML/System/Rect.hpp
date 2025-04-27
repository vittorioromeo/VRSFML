#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility class for manipulating 2D axis-aligned rectangles.
/// \ingroup system
///
/// A rectangle is defined by its top-left corner (`position`) and its
/// dimensions (`size`). It is primarily used for defining regions,
/// bounding boxes, or texture coordinates.
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Rect
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Check if a point is inside the rectangle's area
    ///
    /// This check is non-inclusive. If the point lies on the
    /// edge of the rectangle, this function will return `false`.
    ///
    /// \param point Point to test
    ///
    /// \return `true` if the point is inside, `false` otherwise
    ///
    /// \see `findIntersection`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr bool contains(const Vector2<T> point) const
    {
        // Rectangles with negative dimensions are allowed, so we must handle them correctly

        // Compute the real min and max of the rectangle on both axes
        const T minX = base::min(position.x, static_cast<T>(position.x + size.x));
        const T maxX = base::max(position.x, static_cast<T>(position.x + size.x));
        const T minY = base::min(position.y, static_cast<T>(position.y + size.y));
        const T maxY = base::max(position.y, static_cast<T>(position.y + size.y));

        return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Rect` of type `OtherRect`
    ///
    /// `OtherRect` must be a `Rect<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename U>
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr U to() const
    {
        using ValueType = decltype(U{}.position.x);
        return Rect<ValueType>{position.template to<Vector2<ValueType>>(), size.template to<Vector2<ValueType>>()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==`
    ///
    /// This operator compares strict equality between two rectangles.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if \a lhs is equal to \a rhs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const Rect<T>& rhs) const = default;


    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of a specific anchor point within the rectangle
    ///
    /// Calculates the world coordinates of a point based on normalized factors relative
    /// to the rectangle's position and size. `(0, 0)` is the top-left corner,
    /// `(1, 1)` is the bottom-right corner, `(0.5, 0.5)` is the center, etc.
    ///
    /// \param factors Normalized factors `(x, y)` in the range `[0, 1]`.
    ///
    /// \return Coordinates of the calculated anchor point
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2<T> getAnchorPoint(const Vector2f factors) const
    {
        if constexpr (SFML_BASE_IS_SAME(T, float))
        {
            return position + size.componentWiseMul(factors);
        }
        else
        {
            return position + size.toVector2f().componentWiseMul(factors).template to<Vector2<T>>();
        }
    }


    ////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(name, ...)                                           \
    /** \brief Get the position of the name anchor point */                                      \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2<T> name() const \
    {                                                                                            \
        return getAnchorPoint(__VA_ARGS__);                                                      \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER


    ////////////////////////////////////////////////////////////
    /// \brief Calculate the offset required to move the top-left corner
    ///        so that a given anchor point aligns with the original top-left.
    ///
    /// This is useful for positioning the rectangle relative to one of its anchor points.
    ///
    /// \param factors Normalized factors `(x, y)` defining the anchor point.
    ///
    /// \return The offset vector to apply to the rectangle's position.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2<T> getAnchorPointOffset(const Vector2f factors) const
    {
        return -(size.toVector2f().componentWiseMul(factors).template to<Vector2<T>>());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the X coordinate of the left edge.
    ///
    /// Equivalent to `position.x`.
    ///
    /// \return The X coordinate of the left edge.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getLeft() const
    {
        return position.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the X coordinate of the right edge.
    ///
    /// Equivalent to `position.x + size.x`.
    ///
    /// \return The X coordinate of the right edge.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getRight() const
    {
        return position.x + size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the Y coordinate of the top edge.
    ///
    /// Equivalent to `position.y`.
    ///
    /// \return The Y coordinate of the top edge.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getTop() const
    {
        return position.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the Y coordinate of the bottom edge.
    ///
    /// Equivalent to `position.y + size.y`.
    ///
    /// \return The Y coordinate of the bottom edge.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getBottom() const
    {
        return position.y + size.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the rectangle's position based on an anchor point.
    ///
    /// Moves the rectangle so that the anchor point specified by `factors`
    /// aligns with the given `newPosition`.
    ///
    /// \param factors     Normalized factors `(x, y)` defining the anchor point.
    /// \param newPosition Target coordinates for the anchor point.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setAnchorPoint(const Vector2f factors, const Vector2<T> newPosition)
    {
        position = newPosition + getAnchorPointOffset(factors);
    }

    ////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(name, ...)                                \
    /** \brief Set the position based on the name anchor point */                     \
    [[gnu::always_inline, gnu::flatten]] constexpr void name(const Vector2<T> newPos) \
    {                                                                                 \
        return setAnchorPoint(__VA_ARGS__, newPos);                                   \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER

    ////////////////////////////////////////////////////////////
    /// \brief Set the world X coordinate of the left edge.
    ///
    /// This directly sets the `position.x` component.
    ///
    /// \param newCoordinate The new X coordinate for the left edge.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setLeft(const T newCoordinate)
    {
        position.x = newCoordinate;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the world X coordinate of the right edge.
    ///
    /// This directly sets the `position.x` component.
    ///
    /// \param newCoordinate The new X coordinate for the right edge.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setRight(const T newCoordinate)
    {
        position.x = newCoordinate - size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the world Y coordinate of the top edge.
    ///
    /// This directly sets the `position.y` component.
    ///
    /// \param newCoordinate The new Y coordinate for the top edge.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setTop(const T newCoordinate)
    {
        position.y = newCoordinate;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the world Y coordinate of the bottom edge.
    ///
    /// This directly sets the `position.y` component.
    ///
    /// \param newCoordinate The new Y coordinate for the bottom edge.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setBottom(const T newCoordinate)
    {
        position.y = newCoordinate - size.y;
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vector2<T> position; //!< Position of the top-left corner of the rectangle
    Vector2<T> size;     //!< Size of the rectangle
};

// Create type aliases for the most common types
using IntRect   = Rect<int>;
using FloatRect = Rect<float>;
using UIntRect  = Rect<unsigned int>;

} // namespace sf


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
/// (`Vector2` and `Vector3`).
///
/// To keep things simple, `sf::Rect` doesn't define
/// functions to emulate the properties that are not directly
/// members (such as right, bottom, etc.), it rather
/// only provides intersection functions.
///
/// `sf::Rect` uses the usual rules for its boundaries:
/// \li The left and top edges are included in the rectangle's area
/// \li The right and bottom edges are excluded from the rectangle's area
///
/// This means that `sf::IntRect({0, 0}, {1, 1})` and `sf::IntRect({1, 1}, {1, 1})`
/// don't intersect.
///
/// `sf::Rect` is a template and may be used with any numeric type, but
/// for simplicity type aliases for the instantiations used by SFML are given:
/// \li `sf::Rect<int>` is `sf::IntRect`
/// \li `sf::Rect<float>` is `sf::FloatRect`
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
/// sf::base::Optional<sf::IntRect> result = sf::findIntersection(r1, r2);
/// // result.hasValue() == true
/// // result.value() == sf::IntRect({4, 2}, {16, 3})
/// \endcode
///
////////////////////////////////////////////////////////////
