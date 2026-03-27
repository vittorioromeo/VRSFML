#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/NinePatchUtils.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"

#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// \brief Transformable nine-patch rectangle helper.
///
/// The source texture rectangle is split into a 3x3 grid. Corners keep
/// their original proportions, edges stretch along one axis, and the
/// center stretches on both axes.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] NinePatchRect :
    sf::TransformableMixin<NinePatchRect>,
    sf::GlobalAnchorPointMixin<NinePatchRect>,
    sf::LocalAnchorPointMixin<NinePatchRect>
{
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;


    ////////////////////////////////////////////////////////////
    sf::Vec2f        size{};
    sf::Rect2f       textureRect{};
    NinePatchBorders borders{};
    sf::Color        color{sf::Color::White};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr sf::Rect2f getLocalBounds() const
    {
        return {{0.f, 0.f}, size};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline sf::Rect2f getGlobalBounds() const
    {
        return getTransform().transformRect(getLocalBounds());
    }


    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& target, const sf::Texture& texture, sf::RenderStates states = {}) const
    {
        if (size.x <= 0.f || size.y <= 0.f)
            return;

        const sf::Rect2f sourceRect = (textureRect == sf::Rect2f{}) ? texture.getRect() : textureRect;

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

        for (sf::base::SizeT iy = 0; iy < 3u; ++iy)
        {
            for (sf::base::SizeT ix = 0; ix < 3u; ++ix)
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
    void drawPatch(sf::RenderTarget&       target,
                   const sf::Texture&      texture,
                   const sf::RenderStates& states,
                   const sf::Rect2f        source,
                   const sf::Rect2f        destination) const
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
