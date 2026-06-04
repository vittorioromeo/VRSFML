#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Export.hpp"

#include "Zancle/Graphics/Priv/ShapeMacros.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"

#include "Zancle/System/GlobalAnchorPointMixin.hpp"
#include "Zancle/System/LocalAnchorPointMixin.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/MinMaxMacros.hpp"
#include "ZancleBase/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Defines the geometric properties of an arrow shape
///
/// `za::ArrowShapeData` is used to specify the dimensions and
/// characteristics of an arrow, such as the length and width
/// of its shaft and head. This data is then typically used by
/// a rendering system or a shape class to draw the actual arrow.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API ArrowShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float shaftLength{50.f}; //!< Length of the shaft of the arrow
    float shaftWidth{10.f};  //!< Width of the shaft of the arrow
    float headLength{20.f};  //!< Length of the head of the arrow
    float headWidth{30.f};   //!< Width of the head of the arrow


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the arrow would actually render anything.
    ///
    /// An arrow with negative dimensions, zero total length
    /// (`shaftLength + headLength`), or zero thickness
    /// (`max(shaftWidth, headWidth)`) produces no geometry and is
    /// treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return shaftLength >= 0.f && shaftWidth >= 0.f && headLength >= 0.f && headWidth >= 0.f &&
               (shaftLength + headLength) > 0.f && ZB_MAX(shaftWidth, headWidth) > 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the arrow.
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        const float maxW = ZB_MAX(shaftWidth, headWidth);
        return {{0.f, -maxW * 0.5f}, {shaftLength + headLength, maxW}};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the arrow.
    ///
    /// Folds the seven actual arrow vertices through the shape
    /// transform. Unlike `transformRect(localBounds)`, this does
    /// not over-count the empty corners of the local bbox when the
    /// arrow is rotated.
    ///
    /// \par Cost
    /// **Vertex iteration, O(1)** -- the arrow has a fixed 7
    /// vertices, so seven point transforms and an AABB fold. Cheap
    /// in absolute terms, but technically a per-vertex loop rather
    /// than a closed-form formula.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the arrow.
    ///
    /// The arrow is symmetric about `y = 0`, so `Cy = 0`. It
    /// decomposes cleanly into a shaft rectangle
    /// (`shaftLength * shaftWidth`, centroid x = `shaftLength / 2`)
    /// and a head triangle (`0.5 * headLength * headWidth`, centroid
    /// x = `shaftLength + headLength / 3`). The area-weighted
    /// combination yields `Cx` in closed form.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- a handful of multiplies and one
    /// division.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        const float shaftArea = shaftLength * shaftWidth;
        const float headArea  = 0.5f * headLength * headWidth;
        const float totalArea = shaftArea + headArea;

        if (totalArea <= 0.f) [[unlikely]]
            return {(shaftLength + headLength) * 0.5f, 0.f};

        const float shaftCx = shaftLength * 0.5f;
        const float headCx  = shaftLength + headLength * (1.f / 3.f);

        return {(shaftArea * shaftCx + headArea * headCx) / totalArea, 0.f};
    }
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::ArrowShapeData
/// \ingroup graphics
///
/// `za::ArrowShapeData` is a plain struct that holds the geometric
/// definition of an arrow: the length/width of the shaft and the
/// length/width of the head. It also embeds the standard
/// transformable members (`position`, `scale`, `origin`, `rotation`)
/// and the standard shape appearance members (`textureRect`,
/// `outlineTextureRect`, `fillColor`, `outlineColor`,
/// `outlineThickness`, `miterLimit`).
///
/// `ArrowShapeData` is consumed by `za::ShapeUtils` and by
/// `za::CPUDrawableBatch::add` to generate the corresponding vertex
/// data. It is not directly drawable on its own.
///
/// Example usage:
/// \code
/// const za::ArrowShapeData arrowData{
///     .position    = {100.f, 100.f},
///     .fillColor   = za::Color::White,
///     .shaftLength = 100.f,
///     .shaftWidth  = 5.f,
///     .headLength  = 20.f,
///     .headWidth   = 15.f,
/// };
///
/// drawableBatch.add(arrowData);
/// \endcode
///
/// \see `za::Shape`, `za::ShapeUtils`, `za::CurvedArrowShapeData`, `za::DrawableBatch`
///
////////////////////////////////////////////////////////////
