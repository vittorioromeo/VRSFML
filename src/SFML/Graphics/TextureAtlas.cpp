#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vector2.hpp"

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
base::Optional<Vector2f> TextureAtlas::add(const std::uint8_t* pixels, Vector2u size)
{
    const auto packedPosition = m_rectPacker.pack(size);

    if (!packedPosition.hasValue())
        return fail("pack pixel array rectangle for texture atlas");

    m_atlasTexture.update(pixels, size, *packedPosition);

    return base::makeOptional(packedPosition->to<Vector2f>());
}


////////////////////////////////////////////////////////////
base::Optional<Vector2f> TextureAtlas::add(const Image& image)
{
    return add(image.getPixelsPtr(), image.getSize());
}


////////////////////////////////////////////////////////////
base::Optional<Vector2f> TextureAtlas::add(const Texture& texture)
{
    const auto packedPosition = m_rectPacker.pack(texture.getSize());

    if (!packedPosition.hasValue())
        return fail("pack texture rectangle for texture atlas");

    if (!m_atlasTexture.update(texture, *packedPosition))
        return fail("update texture for texture atlas");

    return base::makeOptional(packedPosition->to<Vector2f>());
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
