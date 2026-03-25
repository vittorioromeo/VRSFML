#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
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

#include "SFML/Base/Array.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// \brief Border thickness configuration for a nine-patch rectangle.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] NinePatchBorders
{
    ////////////////////////////////////////////////////////////
    float left{};
    float right{};
    float top{};
    float bottom{};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr NinePatchBorders all(const float value) noexcept
    {
        return {value, value, value, value};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr NinePatchBorders symmetric(
        const float horizontal,
        const float vertical) noexcept
    {
        return {horizontal, horizontal, vertical, vertical};
    }
};


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

        const auto srcX = makeSlices(sourceRect.size.x, borders.left, borders.right);
        const auto srcY = makeSlices(sourceRect.size.y, borders.top, borders.bottom);
        const auto dstX = makeSlices(size.x, borders.left, borders.right);
        const auto dstY = makeSlices(size.y, borders.top, borders.bottom);

        const sf::base::Array<float, 3> srcPosX{
            sourceRect.position.x,
            sourceRect.position.x + srcX[0],
            sourceRect.position.x + srcX[0] + srcX[1],
        };

        const sf::base::Array<float, 3> srcPosY{
            sourceRect.position.y,
            sourceRect.position.y + srcY[0],
            sourceRect.position.y + srcY[0] + srcY[1],
        };

        const sf::base::Array<float, 3> dstPosX{0.f, dstX[0], dstX[0] + dstX[1]};
        const sf::base::Array<float, 3> dstPosY{0.f, dstY[0], dstY[0] + dstY[1]};

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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr float clampNonNegative(const float value) noexcept
    {
        return value < 0.f ? 0.f : value;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr sf::base::Array<float, 3> makeSlices(
        const float total,
        const float start,
        const float end) noexcept
    {
        const float safeTotal = clampNonNegative(total);
        float       first     = clampNonNegative(start);
        float       third     = clampNonNegative(end);

        const float borderSum = first + third;

        if (borderSum > safeTotal && borderSum > 0.f)
        {
            const float factor = safeTotal / borderSum;
            first *= factor;
            third *= factor;
        }

        return {first, safeTotal - first - third, third};
    }


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
