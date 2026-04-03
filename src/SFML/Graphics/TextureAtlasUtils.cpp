// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureAtlasUtils.hpp"

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::NullOpt fail(const char* what)
{
    sf::priv::err() << "Failed to " << what;
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
    const auto packedPosition = rectPacker.pack(size + padding);

    if (!packedPosition.hasValue())
        return fail("pack pixel array rectangle for texture atlas");

    targetTexture.update(pixels, size, *packedPosition);

    return base::makeOptional<Rect2f>(packedPosition->to<Vec2f>(), size.to<Vec2f>());
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Image& image)
{
    return add(targetTexture, rectPacker, padding, image.getPixelsPtr(), image.getSize());
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Texture& texture)
{
    const auto packedPosition = rectPacker.pack(texture.getSize() + padding);

    if (!packedPosition.hasValue())
        return fail("pack texture rectangle for texture atlas");

    if (!targetTexture.update(texture, *packedPosition))
        return fail("update texture for texture atlas");

    return base::makeOptional<Rect2f>(packedPosition->to<Vec2f>(), texture.getSize().to<Vec2f>());
}

} // namespace sf
