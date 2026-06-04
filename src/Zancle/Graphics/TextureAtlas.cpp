// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TextureAtlas.hpp"
#include "Zancle/Graphics/TextureAtlasUtils.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/RectPacker.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"


namespace za
{
////////////////////////////////////////////////////////////
TextureAtlas::TextureAtlas(Texture&& atlasTexture) :
    m_atlasTexture(ZB_MOVE(atlasTexture)),
    m_rectPacker(m_atlasTexture.getSize())
{
}


////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlas::add(const zb::U8* pixels, Vec2u size, Vec2u padding)
{
    return TextureAtlasUtils::add(m_atlasTexture, m_rectPacker, padding, pixels, size);
}


////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlas::add(const Image& image, Vec2u padding)
{
    return TextureAtlasUtils::add(m_atlasTexture, m_rectPacker, padding, image);
}


////////////////////////////////////////////////////////////
zb::Optional<Rect2f> TextureAtlas::add(const Texture& texture, Vec2u padding)
{
    return TextureAtlasUtils::add(m_atlasTexture, m_rectPacker, padding, texture);
}


////////////////////////////////////////////////////////////
Texture& TextureAtlas::getTexture()
{
    return m_atlasTexture;
}


////////////////////////////////////////////////////////////
const Texture& TextureAtlas::getTexture() const
{
    return m_atlasTexture;
}


////////////////////////////////////////////////////////////
RectPacker& TextureAtlas::getRectPacker()
{
    return m_rectPacker;
}


////////////////////////////////////////////////////////////
const RectPacker& TextureAtlas::getRectPacker() const
{
    return m_rectPacker;
}

} // namespace za
