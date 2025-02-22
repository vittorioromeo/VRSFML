#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp" // used, exposes macros
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/AnchorPointMixin.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Span.hpp"
#include "SFML/Base/TrivialVector.hpp"


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
class SFML_GRAPHICS_API Shape : public Transformable, public AnchorPointMixin<Shape>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Settings
    {
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    explicit Shape(const Settings& settings);

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
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, ...) that are applied to the entity.
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
    /// rotation, scale, ...) that are applied to the entity.
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
    [[nodiscard]] base::Span<const Vertex> getFillVertices() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Span<const Vertex> getOutlineVertices() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Recompute the internal geometry of the shape
    ///
    /// This function must be called by the derived class every time
    /// the shape's points change (i.e. the result of either
    /// getPointCount or getPoint is different).
    ///
    ////////////////////////////////////////////////////////////
    void update(const sf::Vector2f* points, base::SizeT pointCount);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateImplResizeVerticesVector(base::SizeT pointCount);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void updateImplFromVerticesPositions(base::SizeT pointCount, bool mustUpdateBounds);

private:
    friend RenderTarget;

    template <typename TStorage>
    friend class priv::DrawableBatchImpl;

    ////////////////////////////////////////////////////////////
    /// \brief Draws the shape on `renderTarget` with the given `texture` and `states`
    ///
    ////////////////////////////////////////////////////////////
    void drawOnto(RenderTarget& renderTarget, const Texture* texture, RenderStates states) const;

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
    void updateOutline(bool mustUpdateBounds);

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' color
    ///
    ////////////////////////////////////////////////////////////
    void updateOutlineColors();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    FloatRect m_textureRect{};              //!< Area of the source texture to display for the fill
    FloatRect m_outlineTextureRect{};       //!< Area of the source texture to display for the outline
    Color     m_fillColor{Color::White};    //!< Fill color
    Color     m_outlineColor{Color::White}; //!< Outline color
    float     m_outlineThickness{};         //!< Thickness of the shape's outline

protected:
    base::TrivialVector<Vertex> m_vertices; //!< Vertex array containing the fill and outline geometry

private:
    base::SizeT m_verticesEndIndex = 0u; //!< Index where the fill vertices end and outline vertices begin
    FloatRect   m_insideBounds;          //!< Bounding rectangle of the inside (fill)
    FloatRect   m_bounds;                //!< Bounding rectangle of the whole shape (outline + fill)
};

} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename TSettings>
[[nodiscard]] inline Shape::Settings toShapeSettings(const TSettings& settings)
{
    return {
        .position           = settings.position,
        .scale              = settings.scale,
        .origin             = settings.origin,
        .rotation           = settings.rotation,
        .textureRect        = settings.textureRect,
        .outlineTextureRect = settings.outlineTextureRect,
        .fillColor          = settings.fillColor,
        .outlineColor       = settings.outlineColor,
        .outlineThickness   = settings.outlineThickness,
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
