// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlasUtils.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
TextureAtlas::TextureAtlas(Texture&& atlasTexture) :
    m_atlasTexture(SFML_BASE_MOVE(atlasTexture)),
    m_rectPacker(m_atlasTexture.getSize())
{
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlas::add(const base::U8* pixels, Vec2u size, Vec2u padding)
{
    return TextureAtlasUtils::add(m_atlasTexture, m_rectPacker, padding, pixels, size);
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlas::add(const Image& image, Vec2u padding)
{
    return TextureAtlasUtils::add(m_atlasTexture, m_rectPacker, padding, image);
}


////////////////////////////////////////////////////////////
base::Optional<Rect2f> TextureAtlas::add(const Texture& texture, Vec2u padding)
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

} // namespace sf
