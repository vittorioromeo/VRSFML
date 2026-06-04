#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/NinePatchUtils.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TransformableMixin.hpp"

#include "Zancle/System/GlobalAnchorPointMixin.hpp"
#include "Zancle/System/LocalAnchorPointMixin.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
/// \brief Transformable nine-patch rectangle helper.
///
/// The source texture rectangle is split into a 3x3 grid. Corners keep
/// their original proportions, edges stretch along one axis, and the
/// center stretches on both axes.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] NinePatchRect : za::TransformableMixin, za::GlobalAnchorPointMixin, za::LocalAnchorPointMixin
{
    ////////////////////////////////////////////////////////////
    ZA_DEFINE_TRANSFORMABLE_DATA_MEMBERS;


    ////////////////////////////////////////////////////////////
    za::Vec2f        size{};
    za::Rect2f       textureRect{};
    NinePatchBorders borders{};
    za::Color        color{za::Color::White};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr za::Rect2f getLocalBounds() const
    {
        return {{0.f, 0.f}, size};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline za::Rect2f getGlobalBounds() const
    {
        return getTransform().transformRect(getLocalBounds());
    }


    ////////////////////////////////////////////////////////////
    void draw(za::RenderTarget& target, const za::Texture& texture, za::RenderStates states = {}) const
    {
        if (size.x <= 0.f || size.y <= 0.f)
            return;

        const za::Rect2f sourceRect = (textureRect == za::Rect2f{}) ? texture.getRect() : textureRect;

        if (sourceRect.size.x <= 0.f || sourceRect.size.y <= 0.f)
            return;

        const auto srcX    = makeNinePatchSlices(sourceRect.size.x, borders.left, borders.right);
        const auto srcY    = makeNinePatchSlices(sourceRect.size.y, borders.top, borders.bottom);
        const auto dstX    = makeNinePatchSlices(size.x, borders.left, borders.right);
        const auto dstY    = makeNinePatchSlices(size.y, borders.top, borders.bottom);
        const auto srcPosX = makeNinePatchPositions(sourceRect.position.x, srcX);
        const auto srcPosY = makeNinePatchPositions(sourceRect.position.y, srcY);
        const auto dstPosX = makeNinePatchPositions(0.f, dstX);
        const auto dstPosY = makeNinePatchPositions(0.f, dstY);

        states.transform *= getTransform();

        for (zb::SizeT iy = 0; iy < 3u; ++iy)
        {
            for (zb::SizeT ix = 0; ix < 3u; ++ix)
            {
                drawPatch(target,
                          texture,
                          states,
                          {{srcPosX[ix], srcPosY[iy]}, {srcX[ix], srcY[iy]}},
                          {{dstPosX[ix], dstPosY[iy]}, {dstX[ix], dstY[iy]}});
            }
        }
    }

private:
    ////////////////////////////////////////////////////////////
    void drawPatch(za::RenderTarget&       target,
                   const za::Texture&      texture,
                   const za::RenderStates& states,
                   const za::Rect2f        source,
                   const za::Rect2f        destination) const
    {
        if (source.size.x <= 0.f || source.size.y <= 0.f || destination.size.x <= 0.f || destination.size.y <= 0.f)
            return;

        target.draw(texture,
                    {
                        .position    = destination.position,
                        .scale       = {destination.size.x / source.size.x, destination.size.y / source.size.y},
                        .textureRect = source,
                        .color       = color,
                    },
                    states);
    }
};
