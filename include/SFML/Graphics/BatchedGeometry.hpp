#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VertexSpan.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Result of a batched geometry generation operation
///
/// Returned by `sf::DrawableBatch::add` and `sf::RenderTarget::draw`
/// overloads that generate vertex data from a high-level description
/// (e.g., `sf::CircleShapeData`, `sf::TextData`). It exposes the
/// newly-emitted vertices split into their semantic regions so that
/// callers can inspect or mutate them (color, position, ...) before
/// the batch is flushed to the GPU.
///
/// The two spans are independent sub-ranges of the batch's vertex
/// storage. Either may be empty:
/// \li `fill` is empty if the shape has no visible fill geometry
///     (e.g., fully transparent fill, or degenerate size).
/// \li `outline` is empty if the shape has no outline
///     (e.g., `outlineThickness == 0`), or for drawables that do not
///     produce a separate outline region (sprites, raw vertices).
///
/// \warning The spans are invalidated by the next `add`/`draw`
///          call on the same batch, by `clear`, or by any operation
///          that may reallocate the underlying storage. Copy out the
///          data if you need to keep it.
///
/// \see sf::VertexSpan, sf::DrawableBatch, sf::RenderTarget
///
////////////////////////////////////////////////////////////
struct BatchedGeometry
{
    VertexSpan fill;    //!< Vertices that make up the shape's fill (may be empty)
    VertexSpan outline; //!< Vertices that make up the shape's outline (may be empty)
};

} // namespace sf
