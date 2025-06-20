#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Image;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API TextureAtlas
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TextureAtlas(Texture&& atlasTexture);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<FloatRect> add(const base::U8* pixels, Vec2u size, Vec2u padding = {});
    [[nodiscard]] base::Optional<FloatRect> add(const Image& image, Vec2u padding = {});
    [[nodiscard]] base::Optional<FloatRect> add(const Texture& texture, Vec2u padding = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Texture&       getTexture();
    [[nodiscard]] const Texture& getTexture() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] RectPacker&       getRectPacker();
    [[nodiscard]] const RectPacker& getRectPacker() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Texture    m_atlasTexture;
    RectPacker m_rectPacker;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TextureAtlas
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::Texture, sf::Image, sf::RenderTexture
///
////////////////////////////////////////////////////////////
