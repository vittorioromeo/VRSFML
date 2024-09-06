#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
TextureAtlas::TextureAtlas(Texture& atlasTexture) :
m_atlasTexturePtr(&atlasTexture),
m_rectPacker(atlasTexture.getSize())
{
}


////////////////////////////////////////////////////////////
base::Optional<Vector2f> TextureAtlas::add(const Texture& texture)
{
    const auto packedPosition = m_rectPacker.pack(texture.getSize());
    if (!packedPosition.hasValue())
    {
        priv::err() << "Failed to pack texture rectangle for texture atlas";
        return base::nullOpt;
    }

    if (!m_atlasTexturePtr->update(texture, *packedPosition))
    {
        priv::err() << "Failed to update texture for texture atlas";
        return base::nullOpt;
    }

    return base::makeOptional(packedPosition->to<Vector2f>());
}

} // namespace sf
