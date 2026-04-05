#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Glyph.hpp"

#include "SFML/System/LifetimeDependee.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class GlyphMappedText;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Settings for loading glyphs via FontFace::loadGlyphs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] GlyphLoadSettings
{
    const char32_t* codePoints{};
    base::SizeT     codePointCount{}; // TODO P1: span?

    unsigned int characterSize{30u};
    bool         bold{false};
    float        outlineThickness{0.f};

    ////////////////////////////////////////////////////////////
    /// \brief Create settings for printable ASCII (U+0020 to U+007E)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static GlyphLoadSettings ascii(unsigned int characterSize, bool bold, float outlineThickness);

    ////////////////////////////////////////////////////////////
    /// \brief Create settings for printable ASCII + Latin-1 Supplement (U+0020 to U+00FF)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static GlyphLoadSettings latin1(unsigned int characterSize, bool bold, float outlineThickness);
};


////////////////////////////////////////////////////////////
/// \brief Immutable mapping of code points to preloaded glyphs
///
/// Created by `FontFace::loadGlyphs()`. Contains preloaded glyph
/// metrics and cached font metrics. Does not store references to
/// `FontFace` or `Texture` — those are provided externally.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API GlyphMapping
{
    ////////////////////////////////////////////////////////////
    /// \brief Groups a fill and outline glyph together
    ///
    ////////////////////////////////////////////////////////////
    struct GlyphPair
    {
        const Glyph& fillGlyph;
        const Glyph& outlineGlyph;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Get a glyph from the mapping
    ///
    /// Signature matches Font::getGlyph for TextUtils compatibility.
    /// The characterSize/bold/outlineThickness params are baked in
    /// and only verified via debug asserts.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Glyph& getGlyph(char32_t     codePoint,
                                        unsigned int theCharacterSize,
                                        bool         theBold,
                                        float        theOutlineThickness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get both fill and outline glyphs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] GlyphPair getFillAndOutlineGlyph(char32_t     codePoint,
                                                   unsigned int theCharacterSize,
                                                   bool         theBold,
                                                   float        theOutlineThickness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get line spacing
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getLineSpacing(unsigned int theCharacterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get ascent
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getAscent(unsigned int theCharacterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get descent
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDescent(unsigned int theCharacterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get underline position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlinePosition(unsigned int theCharacterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get underline thickness
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlineThickness(unsigned int theCharacterSize) const;

    ////////////////////////////////////////////////////////////
    // Glyph data
    ////////////////////////////////////////////////////////////
    ankerl::unordered_dense::map<char32_t, Glyph> fillGlyphs;
    ankerl::unordered_dense::map<char32_t, Glyph> outlineGlyphs;

    ////////////////////////////////////////////////////////////
    // Baked-in parameters
    ////////////////////////////////////////////////////////////
    unsigned int characterSize{};
    bool         bold{};
    float        outlineThickness{};

    ////////////////////////////////////////////////////////////
    // Cached font metrics
    ////////////////////////////////////////////////////////////
    float cachedLineSpacing{};
    float cachedAscent{};
    float cachedDescent{};
    float cachedUnderlinePosition{};
    float cachedUnderlineThickness{};

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(GlyphMapping, GlyphMappedText);
};

} // namespace sf
