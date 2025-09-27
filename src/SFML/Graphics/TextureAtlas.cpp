// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2.hpp"

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
TextureAtlas::TextureAtlas(Texture&& atlasTexture) :
    m_atlasTexture(SFML_BASE_MOVE(atlasTexture)),
    m_rectPacker(m_atlasTexture.getSize())
{
}


////////////////////////////////////////////////////////////
base::Optional<FloatRect> TextureAtlas::add(const base::U8* pixels, Vec2u size, Vec2u padding)
{
    const auto packedPosition = m_rectPacker.pack(size + padding);

    if (!packedPosition.hasValue())
        return fail("pack pixel array rectangle for texture atlas");

    m_atlasTexture.update(pixels, size, *packedPosition);

    return base::makeOptional<FloatRect>(packedPosition->to<Vec2f>(), size.to<Vec2f>());
}


////////////////////////////////////////////////////////////
base::Optional<FloatRect> TextureAtlas::add(const Image& image, Vec2u padding)
{
    return add(image.getPixelsPtr(), image.getSize(), padding);
}


////////////////////////////////////////////////////////////
base::Optional<FloatRect> TextureAtlas::add(const Texture& texture, Vec2u padding)
{
    const auto packedPosition = m_rectPacker.pack(texture.getSize() + padding);

    if (!packedPosition.hasValue())
        return fail("pack texture rectangle for texture atlas");

    if (!m_atlasTexture.update(texture, *packedPosition))
        return fail("update texture for texture atlas");

    return base::makeOptional<FloatRect>(packedPosition->to<Vec2f>(), texture.getSize().to<Vec2f>());
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
