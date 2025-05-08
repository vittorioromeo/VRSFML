#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexUtils.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Span.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Implementation of a vertex span, can be mutable or immutable
///
////////////////////////////////////////////////////////////
template <typename T>
struct VertexSpanImpl : base::Span<T>
{
    using base::Span<T>::Span;

    ////////////////////////////////////////////////////////////
    /// \brief Converts a mutable vertex span to an immutable vertex span
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr operator VertexSpanImpl<const T>() const noexcept
    {
        return VertexSpanImpl<const T>{this->theData, this->theSize};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the bounding rectangle of this vertex span.
    ///
    /// This function iterates through a sequence of vertices and determines the
    /// smallest axis-aligned rectangle that encloses all the vertices.
    ///
    /// \return A rect representing the bounding rectangle.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr FloatRect getGlobalBounds() const noexcept
    {
        return VertexUtils::getVertexRangeBounds(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the minimum x-coordinate of this vertex span.
    ///
    /// \return The minimum x-coordinate.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLeft() const noexcept
    {
        return VertexUtils::getLeft(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the maximum x-coordinate of this vertex span.
    ///
    /// \return The maximum x-coordinate.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getRight() const noexcept
    {
        return VertexUtils::getRight(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the minimum y-coordinate of this vertex span.
    ///
    /// \return The minimum y-coordinate.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getTop() const noexcept
    {
        return VertexUtils::getTop(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the maximum y-coordinate of this vertex span.
    ///
    /// \return The maximum y-coordinate.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getBottom() const noexcept
    {
        return VertexUtils::getBottom(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the (min x, min y) of this vertex span.
    ///
    /// \return The (min x, min y) of this vertex span.
    ///
    ////////////////////////////////////////////////////////////D
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getTopLeft() const noexcept
    {
        return VertexUtils::getTopLeft(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the (max x, min y) of this vertex span.
    ///
    /// \return The (max x, min y) of this vertex span.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getTopRight() const noexcept
    {
        return VertexUtils::getTopRight(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the (min y, max y) of this vertex span.
    ///
    /// \return The (min y, max y) of this vertex span.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getBottomLeft() const noexcept
    {
        return VertexUtils::getBottomLeft(this->theData, this->theSize);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Computes the (max x, max y) of this vertex span.
    ///
    /// \return The (max x, max y) of this vertex span.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getBottomRight() const noexcept
    {
        return VertexUtils::getBottomRight(this->theData, this->theSize);
    }
};

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Mutable span of vertices
///
////////////////////////////////////////////////////////////
using VertexSpan = priv::VertexSpanImpl<Vertex>;

////////////////////////////////////////////////////////////
/// \brief Immutable span of vertices
///
////////////////////////////////////////////////////////////
using ConstVertexSpan = priv::VertexSpanImpl<const Vertex>;

} // namespace sf

////////////////////////////////////////////////////////////
/// \struct sf::VertexSpan
/// \ingroup graphics
///
/// The `sf::VertexSpan` class is a refinement of `base::Span`
/// that provides additional functionality for working with
/// vertex data.
///
/// \see `sf::Vertex`, `sf::base::Span`
///
////////////////////////////////////////////////////////////
