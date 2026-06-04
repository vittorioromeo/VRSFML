// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/TextureAtlasUtils.hpp"

#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/Texture.hpp"

#include "Zancle/System/Err.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/RectPacker.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] zb::NullOpt fail(const char* what)
{
    za::priv::errMsg("Failed to {}", what);
    return zb::nullOpt;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const zb::U8* pixels, Vec2u size)
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

    return zb::makeOptional<Rect2f>(uploadPos.to<Vec2f>(), size.to<Vec2f>());
}


////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Image& image)
{
    return add(targetTexture, rectPacker, padding, image.getPixelsPtr(), image.getSize());
}


////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlasUtils::add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Texture& texture)
{
    // See pixel-array overload for the +padding/-padding rationale.
    const auto packedPosition = rectPacker.pack(texture.getSize() + padding * 2u);

    if (!packedPosition.hasValue())
        return fail("pack texture rectangle for texture atlas");

    const Vec2u uploadPos = *packedPosition + padding;

    if (!targetTexture.update(texture, uploadPos))
        return fail("update texture for texture atlas");

    return zb::makeOptional<Rect2f>(uploadPos.to<Vec2f>(), texture.getSize().to<Vec2f>());
}

} // namespace za
