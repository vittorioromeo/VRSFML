#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/MinMaxMacros.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Trait/IsSame.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief 2D axis-aligned rectangle defined by a top-left `position` and a `size`.
/// \ingroup system
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Rect2
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Check if `point` is inside the rectangle's area
    ///
    /// This check is non-inclusive: points on the right or bottom
    /// edge are considered outside the rectangle.
    ///
    /// \see `findIntersection`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr bool contains(const Vec2<T> point) const
    {
        // Rectangles with negative dimensions are allowed, so we must handle them correctly

        const auto right  = static_cast<T>(position.x + size.x);
        const auto bottom = static_cast<T>(position.y + size.y);

        // Compute the real min and max of the rectangle on both axes
        const T minX = ZB_MIN(position.x, right);
        const T maxX = ZB_MAX(position.x, right);
        const T minY = ZB_MIN(position.y, bottom);
        const T maxY = ZB_MAX(position.y, bottom);

        return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Rect2` of type `U` (must be a `Rect2<...>`)
    ///
    ////////////////////////////////////////////////////////////
    template <typename U>
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr U to() const
    {
        using ValueType = decltype(U{}.position.x);
        return Rect2<ValueType>{position.template to<Vec2<ValueType>>(), size.template to<Vec2<ValueType>>()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to `Rect2<int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2<int> toRect2i() const
    {
        return {position.toVec2i(), size.toVec2i()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to `Rect2<float>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2<float> toRect2f() const
    {
        return {position.toVec2f(), size.toVec2f()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to `Rect2<unsigned int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2<unsigned int> toRect2u() const
    {
        return {position.toVec2u(), size.toVec2u()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to `Rect2<zb::SizeT>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2<zb::SizeT> toRect2uz() const
    {
        return {position.toVec2uz(), size.toVec2uz()};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Strict member-wise equality
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const Rect2<T>& rhs) const = default;


    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of a specific anchor point within the rectangle
    ///
    /// `factors` are normalized in the range `[0, 1]`: `(0, 0)` is the top-left,
    /// `(1, 1)` is the bottom-right, `(0.5, 0.5)` is the center.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec2<T> getAnchorPoint(const Vec2f factors) const
    {
        if constexpr (ZB_IS_SAME(T, float))
        {
            return position + size.componentWiseMul(factors);
        }
        else
        {
            return position + size.toVec2f().componentWiseMul(factors).template to<Vec2<T>>();
        }
    }


    ////////////////////////////////////////////////////////////
#define ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(name, ...)                                          \
    /** \brief Get the position of the name anchor point */                                   \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec2<T> name() const \
    {                                                                                         \
        return getAnchorPoint(__VA_ARGS__);                                                   \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER(getBottomRight, {1.f, 1.f});

#undef ZA_PRIV_DEFINE_RECT_ANCHOR_GETTER


    ////////////////////////////////////////////////////////////
    /// \brief Get the offset to apply so that the given anchor lands on the current top-left
    ///
    /// Useful for positioning the rectangle relative to one of its anchor points.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec2<T> getAnchorPointOffset(const Vec2f factors) const
    {
        return -(size.toVec2f().componentWiseMul(factors).template to<Vec2<T>>());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the X coordinate of the left edge (i.e. `position.x`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getLeft() const
    {
        return position.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the X coordinate of the right edge (i.e. `position.x + size.x`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getRight() const
    {
        return position.x + size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the Y coordinate of the top edge (i.e. `position.y`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getTop() const
    {
        return position.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the Y coordinate of the bottom edge (i.e. `position.y + size.y`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr T getBottom() const
    {
        return position.y + size.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the anchor identified by `factors` lands on `newPosition`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setAnchorPoint(const Vec2f factors, const Vec2<T> newPosition)
    {
        position = newPosition + getAnchorPointOffset(factors);
    }


    ////////////////////////////////////////////////////////////
#define ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(name, ...)                               \
    /** \brief Set the position based on the name anchor point */                  \
    [[gnu::always_inline, gnu::flatten]] constexpr void name(const Vec2<T> newPos) \
    {                                                                              \
        return setAnchorPoint(__VA_ARGS__, newPos);                                \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the top-left anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the top-center anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the top-right anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the center-left anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the center anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the center-right anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the bottom-left anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the bottom-center anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Move the rectangle so that the bottom-right anchor lands on `newPos`
    ///
    ////////////////////////////////////////////////////////////
    ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER(setBottomRight, {1.f, 1.f});

#undef ZA_PRIV_DEFINE_RECT_ANCHOR_SETTER


    ////////////////////////////////////////////////////////////
    /// \brief Set the X coordinate of the left edge (assigns `position.x`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setLeft(const T newCoordinate)
    {
        position.x = newCoordinate;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the X coordinate of the right edge (adjusts `position.x` to keep `size.x`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setRight(const T newCoordinate)
    {
        position.x = newCoordinate - size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the Y coordinate of the top edge (assigns `position.y`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setTop(const T newCoordinate)
    {
        position.y = newCoordinate;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the Y coordinate of the bottom edge (adjusts `position.y` to keep `size.y`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void setBottom(const T newCoordinate)
    {
        position.y = newCoordinate - size.y;
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2<T> position; //!< Position of the top-left corner of the rectangle
    Vec2<T> size;     //!< Size of the rectangle
};

// Aliases for the most common types
using Rect2i  = Rect2<int>;
using Rect2f  = Rect2<float>;
using Rect2u  = Rect2<unsigned int>;
using Rect2uz = Rect2<zb::SizeT>;

} // namespace za


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class za::Rect2<float>;
extern template class za::Rect2<double>;
extern template class za::Rect2<long double>;
extern template class za::Rect2<int>;
extern template class za::Rect2<unsigned int>;
extern template class za::Rect2<zb::SizeT>;


////////////////////////////////////////////////////////////
/// \class za::Rect2
/// \ingroup system
///
/// A rectangle is defined by its top-left corner (`position`) and its
/// `size`. Member variables are public and can be accessed directly,
/// just like in `Vec2` and `Vec3`.
///
/// Convenience accessors are provided for the four edges (`getLeft()`,
/// `getTop()`, `getRight()`, `getBottom()` and their setters) and for
/// anchor points (`getCenter()`, `getTopLeft()`, `setBottomRight()`, etc.).
/// Intersection testing is provided by `za::findIntersection`.
///
/// Boundary rules:
/// \li The left and top edges are included in the rectangle's area
/// \li The right and bottom edges are excluded from the rectangle's area
///
/// So `za::Rect2i({0, 0}, {1, 1})` and `za::Rect2i({1, 1}, {1, 1})` don't intersect.
///
/// Type aliases are provided for the common instantiations:
/// \li `za::Rect2i`  -> `za::Rect2<int>`
/// \li `za::Rect2f`  -> `za::Rect2<float>`
/// \li `za::Rect2u`  -> `za::Rect2<unsigned int>`
/// \li `za::Rect2uz` -> `za::Rect2<zb::SizeT>`
///
/// Usage example:
/// \code
/// // Define a rectangle, located at (0, 0) with a size of 20x5
/// za::Rect2i r1({0, 0}, {20, 5});
///
/// // Define another rectangle, located at (4, 2) with a size of 18x10
/// za::Vec2i position(4, 2);
/// za::Vec2i size(18, 10);
/// za::Rect2i r2(position, size);
///
/// // Test intersections with the point (3, 1)
/// bool b1 = r1.contains({3, 1}); // true
/// bool b2 = r2.contains({3, 1}); // false
///
/// // Test the intersection between r1 and r2
/// zb::Optional<za::Rect2i> result = za::findIntersection(r1, r2);
/// // result.hasValue() == true
/// // result.value() == za::Rect2i({4, 2}, {16, 3})
/// \endcode
///
////////////////////////////////////////////////////////////
