#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"

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
/// \brief Owning texture atlas: a texture plus a rectangle packer
///
/// `sf::TextureAtlas` couples a single `sf::Texture` with a
/// `sf::RectPacker` to let multiple sub-images, glyphs, or
/// textures share one GPU texture. Sharing a texture across
/// many drawables is a prerequisite for effective batching:
/// the renderer can issue a single draw call instead of one
/// per source texture.
///
/// `sf::TextureAtlas` owns its texture by value. The atlas
/// texture is created by the caller and moved into the atlas;
/// from then on, the atlas takes care of inserting new
/// sub-images via `add` and tracking their positions through
/// the packer.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API TextureAtlas
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct an atlas around an existing texture
    ///
    /// The atlas takes ownership of `atlasTexture`. The texture
    /// must already be created with the desired size and
    /// settings; the atlas only manages packing into it.
    ///
    /// \param atlasTexture Source texture to wrap (moved into the atlas)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TextureAtlas(Texture&& atlasTexture);

    ////////////////////////////////////////////////////////////
    /// \brief Add a raw RGBA pixel block to the atlas
    ///
    /// Asks the rect packer for a free region of size
    /// `size + 2 * padding`, uploads the pixels into it, and
    /// returns the texture rectangle (excluding the padding) of
    /// the newly added image.
    ///
    /// \param pixels  Pointer to `size.x * size.y` RGBA pixels
    /// \param size    Width and height of the source pixel block
    /// \param padding Padding to leave around the image (each side)
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if the atlas is full
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Rect2f> add(const base::U8* pixels, Vec2u size, Vec2u padding = {});

    ////////////////////////////////////////////////////////////
    /// \brief Add an `sf::Image` to the atlas
    ///
    /// \param image   Source image (RGBA)
    /// \param padding Padding to leave around the image (each side)
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if the atlas is full
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Rect2f> add(const Image& image, Vec2u padding = {});

    ////////////////////////////////////////////////////////////
    /// \brief Add the contents of another `sf::Texture` to the atlas
    ///
    /// Performs a GPU-to-CPU readback on `texture`, then
    /// uploads its pixels into a free region of the atlas. As
    /// the readback is slow, this is best used during loading.
    ///
    /// \param texture Source texture (will be downloaded first)
    /// \param padding Padding to leave around the image (each side)
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if the atlas is full
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Rect2f> add(const Texture& texture, Vec2u padding = {});

    ////////////////////////////////////////////////////////////
    /// \brief Access the underlying atlas texture (mutable)
    ///
    /// \return Reference to the texture managed by this atlas
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Texture& getTexture();

    ////////////////////////////////////////////////////////////
    /// \brief Access the underlying atlas texture (const)
    ///
    /// \return Const reference to the texture managed by this atlas
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Texture& getTexture() const;

    ////////////////////////////////////////////////////////////
    /// \brief Access the rectangle packer used by this atlas (mutable)
    ///
    /// Exposed so that advanced users can interact with the
    /// packer directly (e.g. to query free space or to insert
    /// rectangles without uploading pixels).
    ///
    /// \return Reference to the underlying `sf::RectPacker`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] RectPacker& getRectPacker();

    ////////////////////////////////////////////////////////////
    /// \brief Access the rectangle packer used by this atlas (const)
    ///
    /// \return Const reference to the underlying `sf::RectPacker`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const RectPacker& getRectPacker() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Texture    m_atlasTexture; //!< Underlying atlas texture (owned by value)
    RectPacker m_rectPacker;   //!< Packer used to allocate free regions of the atlas
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TextureAtlas
/// \ingroup graphics
///
/// `sf::TextureAtlas` is the simplest way to share a single
/// `sf::Texture` between many drawables (sprites, shapes, text
/// glyphs, ...). It owns the atlas texture and uses a
/// `sf::RectPacker` to find free regions for new sub-images.
///
/// Pack as much as possible into a single atlas: every
/// drawable that samples from the same texture can be
/// auto-batched by `sf::RenderTarget`, drastically cutting the
/// number of GPU draw calls per frame.
///
/// Atlases also accept fonts via `sf::Font::openFromFile(...,
/// &atlas)` and `sf::FontFace::loadGlyphs(atlas, ...)`,
/// allowing glyph rasterization to share storage with the rest
/// of your art.
///
/// \see `sf::Texture`, `sf::Image`, `sf::RectPacker`,
///      `sf::Font`, `sf::FontFace`, `sf::TextureAtlasUtils`
///
////////////////////////////////////////////////////////////


// TODO P0: deprecate in favour of textureatlasutils?
