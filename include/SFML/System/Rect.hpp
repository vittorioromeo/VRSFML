#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Traits/IsSame.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility class for manipulating 2D axis aligned rectangles
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
    [[nodiscard]] constexpr bool contains(Vector2<T> point) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Rect` of type `OtherRect`
    ///
    /// `OtherRect` must be a `Rect<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename U>
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr U to() const;

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
    /// \brief TODO P1: docs
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2<T> name() const \
    {                                                                                            \
        return getAnchorPoint(__VA_ARGS__);                                                      \
    }

    // clang-format off
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopLeft,      {0.f,  0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopCenter,    {0.5f, 0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopRight,     {1.f,  0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterLeft,   {0.f,  0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenter,       {0.5f, 0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterRight,  {1.f,  0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomLeft,   {0.f,  1.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomCenter, {0.5f, 1.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomRight,  {1.f,  1.f});
    // clang-format on

#undef SFML_PRIV_DEFINE_RECT_ANCHOR_GETTER

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2<T> getAnchorPointOffset(const Vector2f factors) const
    {
        return -(size.toVector2f().componentWiseMul(factors).template to<Vector2<T>>());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getLeft() const
    {
        return position.x;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getRight() const
    {
        return position.x + size.x;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getTop() const
    {
        return position.y;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getBottom() const
    {
        return position.y + size.y;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setAnchorPoint(const Vector2f factors, const Vector2<T> newPosition)
    {
        position = newPosition + getAnchorPointOffset(factors);
    }

    ////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(name, ...)                                \
    [[gnu::always_inline, gnu::flatten]] constexpr void name(const Vector2<T> newPos) \
    {                                                                                 \
        return setAnchorPoint(__VA_ARGS__, newPos);                                   \
    }

    // clang-format off
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopLeft,      {0.f,  0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopCenter,    {0.5f, 0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopRight,     {1.f,  0.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterLeft,   {0.f,  0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenter,       {0.5f, 0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterRight,  {1.f,  0.5f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomLeft,   {0.f,  1.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomCenter, {0.5f, 1.f});
SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomRight,  {1.f,  1.f});
    // clang-format on

#undef SFML_PRIV_DEFINE_RECT_ANCHOR_SETTER

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setLeft(const T newCoordinate)
    {
        position.x = newCoordinate;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setRight(const T newCoordinate)
    {
        position.x = newCoordinate - size.x;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setTop(const T newCoordinate)
    {
        position.y = newCoordinate;
    }

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

#include "SFML/System/Rect.inl"


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
