#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/LifetimeDependee.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_ANDROID
namespace sf::priv
{
class ResourceStream;
}
#endif

namespace sf
{
class InputStream;
class Path;
class Text;
class Texture;
class TextureAtlas;
struct FontInfo;
struct Glyph;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Class for loading and manipulating character fonts
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Font
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Font();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Font(const Font& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Font(Font&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Font& operator=(const Font& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Font& operator=(Font&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Open the font from a file
    ///
    /// The supported font formats are: TrueType, Type 1, CFF,
    /// OpenType, SFNT, X11 PCF, Windows FNT, BDF, PFR and Type 42.
    /// Note that this function knows nothing about the standard
    /// fonts installed on the user's system, thus you can't
    /// load them directly.
    ///
    /// \warning SFML cannot preload all the font data in this
    /// function, so the file has to remain accessible until
    /// the sf::Font object is destroyed.
    ///
    /// \param filename Path of the font file to load
    ///
    /// \return Font if opening succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromMemory`, `openFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Font> openFromFile(const Path& filename, TextureAtlas* textureAtlas = nullptr);

    ////////////////////////////////////////////////////////////
    /// \brief Open the font from a file in memory
    ///
    /// The supported font formats are: TrueType, Type 1, CFF,
    /// OpenType, SFNT, X11 PCF, Windows FNT, BDF, PFR and Type 42.
    ///
    /// \warning SFML cannot preload all the font data in this
    /// function, so the buffer pointed by `data` has to remain
    /// valid until the `sf::Font` object opens a new font or
    /// is destroyed.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Font if opening succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromFile`, `openFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Font> openFromMemory(const void*   data,
                                                             base::SizeT   sizeInBytes,
                                                             TextureAtlas* textureAtlas = nullptr);

    ////////////////////////////////////////////////////////////
    /// \brief Open the font from a custom stream
    ///
    /// The supported font formats are: TrueType, Type 1, CFF,
    /// OpenType, SFNT, X11 PCF, Windows FNT, BDF, PFR and Type 42.
    ///
    /// \warning SFML cannot preload all the font data in this
    /// function, so the stream has to remain accessible until
    /// the sf::Font object is destroyed.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Font if opening succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromFile`, `openFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Font> openFromStream(InputStream& stream, TextureAtlas* textureAtlas = nullptr);

    ////////////////////////////////////////////////////////////
    /// \brief Get the font information
    ///
    /// \return A structure that holds the font information
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FontInfo& getInfo() const;

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve a glyph of the font
    ///
    /// If the font is a bitmap font, not all character sizes
    /// might be available. If the glyph is not available at the
    /// requested size, an empty glyph is returned.
    ///
    /// You may want to use `hasGlyph` to determine if the
    /// glyph exists before requesting it. If the glyph does not
    /// exist, a font specific default is returned.
    ///
    /// Be aware that using a negative value for the outline
    /// thickness will cause distorted rendering.
    ///
    /// \param codePoint        Unicode code point of the character to get
    /// \param characterSize    Reference character size
    /// \param bold             Retrieve the bold version or the regular one?
    /// \param outlineThickness Thickness of outline (when != 0 the glyph will not be filled)
    ///
    /// \return The glyph corresponding to `codePoint` and `characterSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Glyph& getGlyph(char32_t codePoint, unsigned int characterSize, bool bold, float outlineThickness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Groups a fill and outline glyph together
    ///
    ////////////////////////////////////////////////////////////
    struct GlyphPair
    {
        const Glyph& fillGlyph;    //!< The fill glyph
        const Glyph& outlineGlyph; //!< The outline glyph
    };

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve a pair of fill and outline glyphs of the font
    ///
    /// \see `getGlyph`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] GlyphPair getFillAndOutlineGlyph(char32_t     codePoint,
                                                   unsigned int characterSize,
                                                   bool         bold,
                                                   float        outlineThickness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Determine if this font has a glyph representing the requested code point
    ///
    /// Most fonts only include a very limited selection of glyphs from
    /// specific Unicode subsets, like Latin, Cyrillic, or Asian characters.
    ///
    /// While code points without representation will return a font specific
    /// default character, it might be useful to verify whether specific
    /// code points are included to determine whether a font is suited
    /// to display text in a specific language.
    ///
    /// \param codePoint Unicode code point to check
    ///
    /// \return `true` if the codepoint has a glyph representation, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasGlyph(char32_t codePoint) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the kerning offset of two glyphs
    ///
    /// The kerning is an extra offset (negative) to apply between two
    /// glyphs when rendering them, to make the pair look more "natural".
    /// For example, the pair "AV" have a special kerning to make them
    /// closer than other characters. Most of the glyphs pairs have a
    /// kerning offset of zero, though.
    ///
    /// \param first         Unicode code point of the first character
    /// \param second        Unicode code point of the second character
    /// \param characterSize Reference character size
    /// \param bold          Retrieve the bold version or the regular one?
    ///
    /// \return Kerning value for `first` and `second`, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getKerning(char32_t first, char32_t second, unsigned int characterSize, bool bold = false) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the line spacing
    ///
    /// Line spacing is the vertical offset to apply between two
    /// consecutive lines of text.
    ///
    /// \param characterSize Reference character size
    ///
    /// \return Line spacing, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getLineSpacing(unsigned int characterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the underline
    ///
    /// Underline position is the vertical offset to apply between the
    /// baseline and the underline.
    ///
    /// \param characterSize Reference character size
    ///
    /// \return Underline position, in pixels
    ///
    /// \see `getUnderlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlinePosition(unsigned int characterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the thickness of the underline
    ///
    /// Underline thickness is the vertical size of the underline.
    ///
    /// \param characterSize Reference character size
    ///
    /// \return Underline thickness, in pixels
    ///
    /// \see `getUnderlinePosition`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlineThickness(unsigned int characterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve the texture containing the loaded glyphs of a certain size
    ///
    /// The contents of the returned texture changes as more glyphs
    /// are requested, thus it is not very relevant. It is mainly
    /// used internally by `sf::Text`.
    ///
    /// \return Texture containing the glyphs of the requested size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Texture& getTexture() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable the smooth filter
    ///
    /// When the filter is activated, the font appears smoother
    /// so that pixels are less noticeable. However if you want
    /// the font to look exactly the same as its source file,
    /// you should disable it.
    /// The smooth filter is enabled by default.
    ///
    /// \param smooth `true` to enable smoothing, `false` to disable it
    ///
    /// \see `isSmooth`
    ///
    ////////////////////////////////////////////////////////////
    void setSmooth(bool smooth);

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the smooth filter is enabled or not
    ///
    /// \return `true` if smoothing is enabled, `false` if it is disabled
    ///
    /// \see `setSmooth`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSmooth() const;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Open from stream and print errors with custom message
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Font> openFromStreamImpl(InputStream& stream, TextureAtlas* textureAtlas, const char* type);

    ////////////////////////////////////////////////////////////
    /// \brief Return the index of the internal representation a character
    ///
    /// \param codePoint Unicode code point of the character to load
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getCharIndex(char32_t codePoint) const;

    ////////////////////////////////////////////////////////////
    /// \brief Make sure that the given size is the current one
    ///
    /// \param characterSize Reference character size
    ///
    /// \return `true` on success, `false` if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setCurrentSize(unsigned int characterSize) const;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Create a font from font handles and a family name
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Font(base::PassKey<Font>&&, TextureAtlas* textureAtlas);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(Font, Text);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Font
/// \ingroup graphics
///
/// Fonts can be opened from a file, from memory or from a custom
/// stream, and supports the most common types of fonts. See
/// the openFromFile function for the complete list of supported formats.
///
/// Once it is opened, a `sf::Font` instance provides three
/// types of information about the font:
/// \li Global metrics, such as the line spacing
/// \li Per-glyph metrics, such as bounding box or kerning
/// \li Pixel representation of glyphs
///
/// Fonts alone are not very useful: they hold the font data
/// but cannot make anything useful of it. To do so you need to
/// use the `sf::Text` class, which is able to properly output text
/// with several options such as character size, style, color,
/// position, rotation, etc.
/// This separation allows more flexibility and better performances:
/// indeed a `sf::Font` is a heavy resource, and any operation on it
/// is slow (often too slow for real-time applications). On the other
/// side, a `sf::Text` is a lightweight object which can combine the
/// glyphs data and metrics of a `sf::Font` to display any text on a
/// render target.
/// Note that it is also possible to bind several `sf::Text` instances
/// to the same `sf::Font`.
///
/// It is important to note that the `sf::Text` instance doesn't
/// copy the font that it uses, it only keeps a reference to it.
/// Thus, a `sf::Font` must not be destructed while it is
/// used by a `sf::Text` (i.e. never write a function that
/// uses a local `sf::Font` instance for creating a text).
///
/// Usage example:
/// \code
/// // Open a new font
/// const auto font = sf::Font::openFromFile("arial.ttf").value();
///
/// // Create a text which uses our font
/// sf::Text text1(font);
/// text1.setCharacterSize(30);
/// text1.setStyle(sf::Text::Style::Regular);
///
/// // Create another text using the same font, but with different parameters
/// sf::Text text2(font);
/// text2.setCharacterSize(50);
/// text2.setStyle(sf::Text::Style::Italic);
/// \endcode
///
/// Apart from opening font files, and passing them to instances
/// of `sf::Text`, you should normally not have to deal directly
/// with this class. However, it may be useful to access the
/// font metrics or rasterized glyphs for advanced usage.
///
/// Note that if the font is a bitmap font, it is not scalable,
/// thus not all requested sizes will be available to use. This
/// needs to be taken into consideration when using `sf::Text`.
/// If you need to display text of a certain size, make sure the
/// corresponding bitmap font that supports that size is used.
///
/// \see `sf::Text`
///
////////////////////////////////////////////////////////////
