#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2Base.hpp"

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
/// \brief Stateless helpers that pack pixel data into a texture + rect packer pair
///
/// `TextureAtlasUtils` is the building block of
/// `sf::TextureAtlas`. It exposes the same `add` operations
/// that the atlas does, but as free static functions taking
/// the target texture and the rect packer by reference. This
/// is convenient when you want to manage the storage and the
/// packer separately, or when you do not want the ownership
/// semantics of `sf::TextureAtlas`.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API TextureAtlasUtils
{
    ////////////////////////////////////////////////////////////
    /// \brief Pack a raw RGBA pixel block into the target texture
    ///
    /// Asks `rectPacker` for a free region of size
    /// `size + 2 * padding`, uploads `pixels` into it, and
    /// returns the texture rectangle (excluding the padding) of
    /// the newly added image.
    ///
    /// \param targetTexture Texture into which the pixels are uploaded
    /// \param rectPacker    Rect packer used to allocate free regions of `targetTexture`
    /// \param padding       Padding to leave around the image (each side)
    /// \param pixels        Pointer to `size.x * size.y` RGBA pixels
    /// \param size          Width and height of the source pixel block
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if no free region was found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(
        Texture&        targetTexture,
        RectPacker&     rectPacker,
        Vec2u           padding,
        const base::U8* pixels,
        Vec2u           size);

    ////////////////////////////////////////////////////////////
    /// \brief Pack the pixels of an `sf::Image` into the target texture
    ///
    /// \param targetTexture Texture into which the image is uploaded
    /// \param rectPacker    Rect packer used to allocate free regions of `targetTexture`
    /// \param padding       Padding to leave around the image (each side)
    /// \param image         Source image (RGBA)
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if no free region was found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Image& image);

    ////////////////////////////////////////////////////////////
    /// \brief Pack the pixels of another `sf::Texture` into the target texture
    ///
    /// Performs a GPU-to-CPU readback on `texture` and uploads
    /// the result into `targetTexture`. As the readback is
    /// slow, this is best used during loading.
    ///
    /// \param targetTexture Texture into which the pixels are uploaded
    /// \param rectPacker    Rect packer used to allocate free regions of `targetTexture`
    /// \param padding       Padding to leave around the image (each side)
    /// \param texture       Source texture (will be downloaded first)
    ///
    /// \return Texture rectangle of the inserted image, or `base::nullOpt` if no free region was found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(Texture&       targetTexture,
                                                    RectPacker&    rectPacker,
                                                    Vec2u          padding,
                                                    const Texture& texture);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::TextureAtlasUtils
/// \ingroup graphics
///
/// `sf::TextureAtlasUtils` is the stateless companion of
/// `sf::TextureAtlas`. It exposes the same packing operations
/// as free static functions, leaving ownership of the texture
/// and the rect packer to the caller.
///
/// Use it directly when you want to drive the packing process
/// without the convenience class -- for example to share a
/// single rect packer between several textures, or to
/// integrate the atlas into your own resource manager.
///
/// \see `sf::TextureAtlas`, `sf::Texture`, `sf::RectPacker`,
///      `sf::Image`
///
////////////////////////////////////////////////////////////
