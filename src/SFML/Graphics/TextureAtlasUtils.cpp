// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureAtlasUtils.hpp"

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::NullOpt fail(const char* what)
{
    sf::priv::errMsg("Failed to {}", what);
    return sf::base::nullOpt;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlasUtils::add(
    Texture&        targetTexture,
    RectPacker&     rectPacker,
    Vec2u           padding,
    const base::U8* pixels,
    Vec2u           size)
{
    // Reserve the full padded region so neighbouring entries are kept at
    // arm's length on every side, then upload the content offset by `padding`
    // so the same number of padding pixels sit between this entry and its
    // top/left and bottom/right neighbours.
    const auto packedPosition = rectPacker.pack(size + padding * 2u);

    if (!packedPosition.hasValue())
        return fail("pack pixel array rectangle for texture atlas");

    const Vec2u uploadPos = *packedPosition + padding;
    targetTexture.update(pixels, size, uploadPos);

    return base::makeOptional<Rect2f>(uploadPos.to<Vec2f>(), size.to<Vec2f>());
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Image& image)
{
    return add(targetTexture, rectPacker, padding, image.getPixelsPtr(), image.getSize());
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Texture& texture)
{
    // See pixel-array overload for the +padding/-padding rationale.
    const auto packedPosition = rectPacker.pack(texture.getSize() + padding * 2u);

    if (!packedPosition.hasValue())
        return fail("pack texture rectangle for texture atlas");

    const Vec2u uploadPos = *packedPosition + padding;

    if (!targetTexture.update(texture, uploadPos))
        return fail("update texture for texture atlas");

    return base::makeOptional<Rect2f>(uploadPos.to<Vec2f>(), texture.getSize().to<Vec2f>());
}

} // namespace sf
