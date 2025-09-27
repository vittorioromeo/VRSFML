#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp" // used, exposes macros
#include "SFML/Graphics/TransformableMixin.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"
#include "SFML/Graphics/VertexUtils.hpp"

#include "SFML/System/AnchorPointMixin.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CPUDrawableBatch;
class RenderTarget;
class Texture;
struct Color;
struct RenderStates;
} // namespace sf


namespace sf::priv
{
template <typename TStorage>
class DrawableBatchImpl;
}

namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Base class for textured shapes with outline
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Shape : public TransformableMixin<Shape>, public AnchorPointMixin<Shape>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Data
    {
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    explicit Shape(const Data& data);

    ////////////////////////////////////////////////////////////
    /// \brief Set the sub-rectangle of the texture that the shape will display
    ///
    /// The texture rect is useful when you don't want to display
    /// the whole texture, but rather a part of it.
    /// By default, the texture rect covers the entire texture.
    ///
    /// \param rect Rectangle defining the region of the texture to display
    ///
    /// \see `getTextureRect`
    ///
    ////////////////////////////////////////////////////////////
    void setTextureRect(const FloatRect& rect);

    ////////////////////////////////////////////////////////////
    /// \brief Set the sub-rectangle of the texture that the shape outline will display
    ///
    /// The texture rect is useful when you don't want to display
    /// the whole texture, but rather a part of it.
    /// By default, the texture rect covers the entire texture.
    ///
    /// \param rect Rectangle defining the region of the texture to display
    ///
    /// \see getOutlineTextureRect
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineTextureRect(const FloatRect& rect);

    ////////////////////////////////////////////////////////////
    /// \brief Set the fill color of the shape
    ///
    /// This color is modulated (multiplied) with the shape's
    /// texture if any. It can be used to colorize the shape,
    /// or change its global opacity.
    /// You can use `sf::Color::Transparent` to make the inside of
    /// the shape transparent, and have the outline alone.
    /// By default, the shape's fill color is opaque white.
    ///
    /// \param color New color of the shape
    ///
    /// \see `getFillColor`, `setOutlineColor`
    ///
    ////////////////////////////////////////////////////////////
    void setFillColor(Color color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the outline color of the shape
    ///
    /// By default, the shape's outline color is opaque white.
    ///
    /// \param color New outline color of the shape
    ///
    /// \see `getOutlineColor`, `setFillColor`
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineColor(Color color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the thickness of the shape's outline
    ///
    /// Note that negative values are allowed (so that the outline
    /// expands towards the center of the shape), and using zero
    /// disables the outline.
    /// By default, the outline thickness is 0.
    ///
    /// \param thickness New outline thickness
    ///
    /// \see `getOutlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineThickness(float thickness);

    ////////////////////////////////////////////////////////////
    /// \brief Set the limit on the ratio between miter length and outline thickness
    ///
    /// Outline segments around each shape corner are joined either
    /// with a miter or a bevel join.
    /// - A miter join is formed by extending outline segments until
    ///   they intersect. The distance between the point of
    ///   intersection and the shape's corner is the miter length.
    /// - A bevel join is formed by connecting outline segments with
    ///   a straight line perpendicular to the corner's bissector.
    ///
    /// The miter limit is used to determine whether ouline segments
    /// around a corner are joined with a bevel or a miter.
    /// When the ratio between the miter length and outline thickness
    /// exceeds the miter limit, a bevel is used instead of a miter.
    ///
    /// The miter limit is linked to the maximum inner angle of a
    /// corner below which a bevel is used by the following formula:
    ///
    /// miterLimit = 1 / sin(angle / 2)
    ///
    /// The miter limit must be greater than or equal to 1.
    /// By default, the miter limit is 4.
    ///
    /// \param miterLimit New miter limit
    ///
    /// \see getMiterLimit
    ///
    ////////////////////////////////////////////////////////////
    void setMiterLimit(float miterLimit);

    ////////////////////////////////////////////////////////////
    /// \brief Get the sub-rectangle of the texture displayed by the shape
    ///
    /// \return Texture rectangle of the shape
    ///
    /// \see `setTextureRect`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FloatRect& getTextureRect() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sub-rectangle of the texture displayed by the shape outline
    ///
    /// \return Texture rectangle of the shape outline
    ///
    /// \see setTextureRect
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FloatRect& getOutlineTextureRect() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the fill color of the shape
    ///
    /// \return Fill color of the shape
    ///
    /// \see `setFillColor`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Color getFillColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline color of the shape
    ///
    /// \return Outline color of the shape
    ///
    /// \see `setOutlineColor`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Color getOutlineColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline thickness of the shape
    ///
    /// \return Outline thickness of the shape
    ///
    /// \see `setOutlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getOutlineThickness() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the limit on the ratio between miter length and outline thickness
    ///
    /// \return Limit on the ratio between miter length and outline thickness
    ///
    /// \see setMiterLimit
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getMiterLimit() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// entity in the entity's coordinate system.
    ///
    /// \return Local bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FloatRect& getLocalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global (non-minimal) bounding rectangle of the entity
    ///
    /// The returned rectangle is in global coordinates, which means
    /// that it takes into account the transformations (translation,
    /// rotation, scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// shape in the global 2D world's coordinate system.
    ///
    /// This function does not necessarily return the _minimal_
    /// bounding rectangle. It merely ensures that the returned
    /// rectangle covers all the vertices (but possibly more).
    /// This allows for a fast approximation of the bounds as a
    /// first check; you may want to use more precise checks
    /// on top of that.
    ///
    /// \return Global bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] FloatRect getGlobalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] ConstVertexSpan getFillVertices() const
    {
        return {m_vertices.data(), m_verticesEndIndex};
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] ConstVertexSpan getOutlineVertices() const
    {
        return {m_vertices.data() + m_verticesEndIndex, m_vertices.size() - m_verticesEndIndex};
    }

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Recompute the internal geometry of the shape
    ///
    /// This function must be called by the derived class every time
    /// the shape's points change (i.e. the result of either
    /// getPointCount or getPoint is different).
    ///
    ////////////////////////////////////////////////////////////
    void update(const sf::Vec2f* points, base::SizeT pointCount); // TODO P1: make public?

    ////////////////////////////////////////////////////////////
    /// \brief Recompute the internal geometry of the shape
    ///
    /// This function must be called by the derived class every time
    /// the shape's points change (i.e. the result of either
    /// getPointCount or getPoint is different).
    ///
    ////////////////////////////////////////////////////////////
    void updateFromFunc(auto&& getPointFunc, const base::SizeT pointCount) // TODO P1: make public?
    {
        if (pointCount < 3u)
        {
            m_vertices.clear();
            m_verticesEndIndex = 0;
            return;
        }

        m_vertices.resize(pointCount + 2u); // +2 for center and repeated first point
        m_verticesEndIndex = pointCount + 2u;

        for (base::SizeT i = 0u; i < pointCount; ++i)
            m_vertices[i + 1u].position = getPointFunc(i);

        m_vertices[pointCount + 1u].position = m_vertices[1].position; // repeated first point

        // Update the bounding rectangle
        m_insideBounds = VertexUtils::getVertexRangeBounds(m_vertices.data() + 1u, m_verticesEndIndex - 1u); // skip center

        // Compute the center and make it the first vertex
        m_vertices[0].position = m_insideBounds.getCenter();

        // Updates
        updateFillColors();
        updateTexCoords();
        updateOutline();
        updateOutlineTexCoords();
    }

private:
    friend RenderTarget;

    template <typename TStorage>
    friend class priv::DrawableBatchImpl;

    ////////////////////////////////////////////////////////////
    /// \brief Update the fill vertices' color
    ///
    ////////////////////////////////////////////////////////////
    void updateFillColors();

    ////////////////////////////////////////////////////////////
    /// \brief Update the fill vertices' texture coordinates
    ///
    ////////////////////////////////////////////////////////////
    void updateTexCoords();

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' texture coordinates
    ///
    ////////////////////////////////////////////////////////////
    void updateOutlineTexCoords();

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' position
    ///
    ////////////////////////////////////////////////////////////
    void updateOutline();

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' color
    ///
    ////////////////////////////////////////////////////////////
    void updateOutlineColors();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    /* Ordered to minimize padding */
    base::Vector<Vertex> m_vertices;              //!< Vertex array containing the fill and outline geometry
    base::SizeT          m_verticesEndIndex = 0u; //!< Index where the fill vertices end and outline vertices begin

    FloatRect m_textureRect{};        //!< Area of the source texture to display for the fill
    FloatRect m_outlineTextureRect{}; //!< Area of the source texture to display for the outline
    FloatRect m_insideBounds;         //!< Bounding rectangle of the inside (fill)
    FloatRect m_bounds;               //!< Bounding rectangle of the whole shape (outline + fill)

    float m_outlineThickness{}; //!< Thickness of the shape's outline
    float m_miterLimit{4.f};    //!< Limit on the ratio between miter length and outline thickness

    Color m_fillColor{Color::White};    //!< Fill color
    Color m_outlineColor{Color::White}; //!< Outline color

public:
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
};

} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename TData>
[[nodiscard]] inline Shape::Data toShapeData(const TData& data)
{
    return {
        .position           = data.position,
        .scale              = data.scale,
        .origin             = data.origin,
        .rotation           = data.rotation,
        .textureRect        = data.textureRect,
        .outlineTextureRect = data.outlineTextureRect,
        .fillColor          = data.fillColor,
        .outlineColor       = data.outlineColor,
        .outlineThickness   = data.outlineThickness,
        .miterLimit         = data.miterLimit,
    };
}

} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \class sf::Shape
/// \ingroup graphics
///
/// `sf::Shape` is a non-polymorphic base class that allows to
/// define and display a custom convex shape on a render target.
///
/// In addition to the attributes provided by the specialized
/// shape classes, a shape always has the following attributes:
/// \li a texture
/// \li a texture rectangle
/// \li a fill color
/// \li an outline color
/// \li an outline thickness
///
/// Each feature is optional, and can be disabled easily:
/// \li the texture can be null
/// \li the fill/outline colors can be `sf::Color::Transparent`
/// \li the outline thickness can be zero
///
/// \see `sf::RectangleShape`, `sf::CircleShape`, `sf::ConvexShape`, `sf::Transformable`
///
////////////////////////////////////////////////////////////

// TODO P1: this should probably be a shapemixin
