#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/TextClusterGrouping.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextDirection.hpp"
#include "SFML/Graphics/TextLineAlignment.hpp"
#include "SFML/Graphics/TextOrientation.hpp"
#include "SFML/Graphics/TextStyle.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/AnchorPointMixin.hpp"
#include "SFML/System/LifetimeDependant.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/EnumClassBitwiseOps.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include <functional> // TODO P0: avoid this dependency


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class RenderTarget;
struct RenderStates;
struct ShapedGlyph;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Graphical text that can be drawn to a render target
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Text : public TransformableMixin<Text>, public AnchorPointMixin<Text>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of the string drawing styles
    ///
    ////////////////////////////////////////////////////////////
    using Style = TextStyle;

    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of the text alignment options
    ///
    ////////////////////////////////////////////////////////////
    using LineAlignment = TextLineAlignment;

    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of text cluster grouping options
    ///
    ////////////////////////////////////////////////////////////
    using ClusterGrouping = TextClusterGrouping;

    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of text direction options
    ///
    ////////////////////////////////////////////////////////////
    using Direction = TextDirection;

    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of text orientation options
    ///
    ////////////////////////////////////////////////////////////
    using Orientation = TextOrientation;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using Data = TextData;

    ////////////////////////////////////////////////////////////
    /// \brief Construct the text from a string, font and size
    ///
    /// Note that if the used font is a bitmap font, it is not
    /// scalable, thus not all requested sizes will be available
    /// to use. This needs to be taken into consideration when
    /// setting the character size. If you need to display text
    /// of a certain size, make sure the corresponding bitmap
    /// font that supports that size is used.
    ///
    /// \param data Data of the text
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Text(const Font& font, const Data& data);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from a temporary font
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Text(const Font&& font, const Data& data) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Text();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Text(const Text&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Text& operator=(const Text&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Text(Text&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Text& operator=(Text&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Set the text's string
    ///
    /// The `string` argument is a `sf::String`, which can
    /// automatically be constructed from standard string types.
    /// So, the following calls are all valid:
    /// \code
    /// text.setString("hello");
    /// text.setString(L"hello");
    /// text.setString(std::string("hello"));
    /// text.setString(std::wstring(L"hello"));
    /// \endcode
    /// A text's string is empty by default.
    ///
    /// \param string New string
    ///
    /// \see `getString`
    ///
    ////////////////////////////////////////////////////////////
    void setString(const String& string);

    ////////////////////////////////////////////////////////////
    /// \brief Set the text's font
    ///
    /// The `font` argument refers to a font that must
    /// exist as long as the text uses it. Indeed, the text
    /// doesn't store its own copy of the font, but rather keeps
    /// a pointer to the one that you passed to this function.
    /// If the font is destroyed and the text tries to
    /// use it, the behavior is undefined.
    ///
    /// \param font New font
    ///
    /// \see `getFont`
    ///
    ////////////////////////////////////////////////////////////
    void setFont(const Font& font);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow setting from a temporary font
    ///
    ////////////////////////////////////////////////////////////
    void setFont(Font&& font) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Set the character size
    ///
    /// The default size is 30.
    ///
    /// Note that if the used font is a bitmap font, it is not
    /// scalable, thus not all requested sizes will be available
    /// to use. This needs to be taken into consideration when
    /// setting the character size. If you need to display text
    /// of a certain size, make sure the corresponding bitmap
    /// font that supports that size is used.
    ///
    /// \param size New character size, in pixels
    ///
    /// \see `getCharacterSize`
    ///
    ////////////////////////////////////////////////////////////
    void setCharacterSize(unsigned int size);

    ////////////////////////////////////////////////////////////
    /// \brief Set the line spacing factor
    ///
    /// The default spacing between lines is defined by the font.
    /// This method enables you to set a factor for the spacing
    /// between lines. By default the line spacing factor is 1.
    ///
    /// \param spacingFactor New line spacing factor
    ///
    /// \see `getLineSpacing`
    ///
    ////////////////////////////////////////////////////////////
    void setLineSpacing(float spacingFactor);

    ////////////////////////////////////////////////////////////
    /// \brief Set the letter spacing factor
    ///
    /// The default spacing between letters is defined by the font.
    /// This factor doesn't directly apply to the existing
    /// spacing between each character, it rather adds a fixed
    /// space between them which is calculated from the font
    /// metrics and the character size.
    /// Note that factors below 1 (including negative numbers) bring
    /// characters closer to each other.
    /// By default the letter spacing factor is 1.
    ///
    /// \param spacingFactor New letter spacing factor
    ///
    /// \see `getLetterSpacing`
    ///
    ////////////////////////////////////////////////////////////
    void setLetterSpacing(float spacingFactor);

    ////////////////////////////////////////////////////////////
    /// \brief Set the text's style
    ///
    /// You can pass a combination of one or more styles, for
    /// example `sf::Text::Style::Bold | sf::Text::Style::Italic`.
    /// The default style is `sf::Text::Style::Regular`.
    ///
    /// \param style New style
    ///
    /// \see `getStyle`
    ///
    ////////////////////////////////////////////////////////////
    void setStyle(Text::Style style);

    ////////////////////////////////////////////////////////////
    /// \brief Set the fill color of the text
    ///
    /// By default, the text's fill color is opaque white.
    /// Setting the fill color to a transparent color with an outline
    /// will cause the outline to be displayed in the fill area of the text.
    ///
    /// \param color New fill color of the text
    ///
    /// \see `getFillColor`
    ///
    ////////////////////////////////////////////////////////////
    void setFillColor(Color color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the outline color of the text
    ///
    /// By default, the text's outline color is opaque black.
    ///
    /// \param color New outline color of the text
    ///
    /// \see `getOutlineColor`
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineColor(Color color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the fill color alpha channel of the text
    ///
    /// \param color New fill color alpha channel of the text
    ///
    /// \see `getFillColorAlpha`
    ///
    ////////////////////////////////////////////////////////////
    void setFillColorAlpha(base::U8 alpha);

    ////////////////////////////////////////////////////////////
    /// \brief Set the outline color alpha channel of the text
    ///
    /// \param color New outline color alpha channel of the text
    ///
    /// \see `getOutlineColorAlpha`
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineColorAlpha(base::U8 alpha);

    ////////////////////////////////////////////////////////////
    /// \brief Set the thickness of the text's outline
    ///
    /// By default, the outline thickness is 0.
    ///
    /// Be aware that using a negative value for the outline
    /// thickness will cause distorted rendering.
    ///
    /// \param thickness New outline thickness, in pixels
    ///
    /// \see `getOutlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineThickness(float thickness);

    ////////////////////////////////////////////////////////////
    /// \brief Set the line alignment for a multi-line text
    ///
    /// By default, the lines will be aligned according to the
    /// direction of the line's script. Left-to-right scripts
    /// will be aligned to the left and right-to-left scripts
    /// will be aligned to the right.
    ///
    /// Forcing alignment will ignore script direction and always
    /// align according to the requested line alignment.
    ///
    /// \param lineAlignment New line alignment
    ///
    /// \see `getLineAlignment`
    ///
    ////////////////////////////////////////////////////////////
    void setLineAlignment(LineAlignment lineAlignment);

    ////////////////////////////////////////////////////////////
    /// \brief Set the text orientation
    ///
    /// By default, the lines will have horizontal orientation.
    ///
    /// Be aware that most fonts don't natively support vertical
    /// orientations. Fonts that are the most likely to natively
    /// support vertical orientations are those whose scripts
    /// also support vertical orientations e.g. east asian scripts.
    ///
    /// If a font does not natively support vertical orientation,
    /// vertical metrics might still be provided for shaping.
    /// In this case, they are very likely to be emulated and might
    /// not result in good visual output.
    ///
    /// Some metrics such as advance and baseline position will
    /// be rotated so they match the vertical axis.
    ///
    /// \param textOrientation New text orientation
    ///
    /// \see `getOrientation`
    ///
    ////////////////////////////////////////////////////////////
    void setOrientation(TextOrientation textOrientation);

    ////////////////////////////////////////////////////////////
    /// \brief Get the text's string
    ///
    /// The returned string is a `sf::String`, which can automatically
    /// be converted to standard string types. So, the following
    /// lines of code are all valid:
    /// \code
    /// sf::String   s1 = text.getString();
    /// std::string  s2 = text.getString();
    /// std::wstring s3 = text.getString();
    /// \endcode
    ///
    /// \return Text's string
    ///
    /// \see `setString`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const String& getString() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the text's font
    ///
    /// The returned reference is const, which means that you
    /// cannot modify the font when you get it from this function.
    ///
    /// \return Reference to the text's font
    ///
    /// \see `setFont`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Font& getFont() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the character size
    ///
    /// \return Size of the characters, in pixels
    ///
    /// \see `setCharacterSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getCharacterSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the letter spacing factor
    ///
    /// \return Size of the letter spacing factor
    ///
    /// \see `setLetterSpacing`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getLetterSpacing() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the line spacing factor
    ///
    /// \return Size of the line spacing factor
    ///
    /// \see `setLineSpacing`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getLineSpacing() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the text's style
    ///
    /// \return Text's style
    ///
    /// \see `setStyle`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Style getStyle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the fill color of the text
    ///
    /// \return Fill color of the text
    ///
    /// \see `setFillColor`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Color getFillColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline color of the text
    ///
    /// \return Outline color of the text
    ///
    /// \see `setOutlineColor`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Color getOutlineColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the fill color alpha channel of the text
    ///
    /// \return Fill color alpha channel of the text
    ///
    /// \see `setFillColorAlpha`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::U8 getFillColorAlpha() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline color alpha channel of the text
    ///
    /// \return Outline color alpha channel of the text
    ///
    /// \see `setOutlineColorAlpha`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::U8 getOutlineColorAlpha() const;
    // TODO P2: add these to shape as well, maybe use a mixin?

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline thickness of the text
    ///
    /// \return Outline thickness of the text, in pixels
    ///
    /// \see `setOutlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getOutlineThickness() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the line alignment for a multi-line text
    ///
    /// \return Line alignment
    ///
    /// \see `setLineAlignment`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] LineAlignment getLineAlignment() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the text orientation
    ///
    /// \return Text orientation
    ///
    /// \see `setOrientation`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Orientation getOrientation() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return a list of shaped glyphs that make up the text
    ///
    /// The result of shaping i.e. positioning individual glyphs
    /// based on the properties of the font and the input text
    /// is a sequence of shaped glyphs that each have a collection
    /// of properties.
    ///
    /// In addition to the glyph information that is available
    /// by looking up a glyph from a font, the glyph position,
    /// glyph cluster ID and direction of the text represented
    /// by the glyph is provided.
    ///
    /// When specifying unicode text, multiple unicode codepoints
    /// might combine to form e.g. a ligature such as æ or
    /// base-and-mark sequence such as é which are composed of
    /// multiple individual glyphs. These combinations are known
    /// as grapheme clusters. When segmenting text into grapheme
    /// clusters, each cluster identifies a complete unit of text
    /// that will be drawn. There are other methods of segmenting
    /// text into clusters e.g. without combining marks.
    /// Character cluster segmentation is used as the default.
    /// A single grapheme can be represented by an individual
    /// codepoint or by a composition of codepoints e.g. an e as
    /// the base and an accent as the mark which together compose
    /// the grapheme é. The cluster groups that result from shaping
    /// depend on whether the input text provides composed
    /// codepoints or decomposed codepoints. This is an advanced
    /// topic known as unicode normalisation.
    ///
    /// When positioning e.g. a cursor within the text, grapheme
    /// clusters can be treated as the basic units of which the
    /// text is composed and not subdivided into their individual
    /// components or glyphs. If positioning of the cursor within
    /// a single grapheme e.g. a ligature is required, a more
    /// fine-grained cluster segmentation algorithm should be used.
    ///
    /// The returned glyph positions are in local coordinates
    /// (translation, rotation, scale and origin are not applied).
    ///
    /// \return List of shaped glyphs that make up the text
    ///
    /// \see `setClusterGrouping`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const base::Vector<ShapedGlyph>& getShapedGlyphs() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the cluster grouping algorithm in use
    ///
    /// \return The cluster grouping algorithm in use
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ClusterGrouping getClusterGrouping() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the cluster grouping algorithm to use
    ///
    /// By default, character cluster grouping is used.
    ///
    /// Character cluster grouping is good enough to be able to
    /// position cursors in most scenarios. If more coarse-grained
    /// grouping is required, grapheme grouping can be selected.
    ///
    /// Cluster grouping can also be disabled if necessary.
    ///
    /// \param clusterGrouping The cluster grouping algorithm to use
    ///
    ////////////////////////////////////////////////////////////
    void setClusterGrouping(ClusterGrouping clusterGrouping);

    ////////////////////////////////////////////////////////////
    /// \brief Callable that is provided with glyph data for pre-processing
    ///
    /// When re-generating the text geometry, shaping will be
    /// performed on the input string using the set font. The
    /// result of shaping is a set of shaped glyphs. Shaped
    /// glyphs are glyphs that have been positioned by the shaper
    /// and whose script direction has also been determined.
    ///
    /// Because multiple input codepoints can be merged into a
    /// single glyph and single codepoints decomposed into multiple
    /// glyphs, the shaper provides a way to map the shaping output
    /// back to the input. When the input string is provided to
    /// the shaper, a monotonically increasing character index is
    /// attached to each input codepoint. If the input string
    /// consists of 10 codepoints, the indices will be 0 to 9.
    ///
    /// After shaping each shaped glyph will be assigned a
    /// cluster value. These cluster values are derived from the
    /// input indices that were provided to the shaper. Because
    /// of the merging and decomposing that happens during shaping,
    /// there isn't a 1 to 1 mapping between input indices and
    /// output cluster values.
    ///
    /// In order to set the glyph properties reliably, they have
    /// to be set based on text segmentation boundaries such as
    /// graphemes, words and sentences. See the corresponding
    /// methods in `sf::String` that can check for these boundaries.
    ///
    /// Once the input text segments to be pre-processed have
    /// been determined, they have to be applied to the shaped
    /// glyphs. When using character or grapheme cluster grouping
    /// it is guaranteed that the resulting cluster values are
    /// monotonic. This means that cluster values will not be
    /// reordered beyond the bounds of the indices that were
    /// provided with the input text.
    ///
    /// What this means is that given a segment of text that
    /// should e.g. be colored differently, if a beginning and
    /// end index can be determined from the input codepoints,
    /// these index boundaries can be used to select the clusters
    /// of the shaped glyphs that correspond to the input segment
    /// and thus whose color needs to be set.
    ///
    /// Here is an example string with codepoint indices:
    /// \code
    /// I   l i k e   f l o w e r s ,   m u f f i n s   a n d   w a f f l e s .
    /// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
    /// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    /// \endcode
    /// After shaping, due to ligature merging of fl, ffi and ffl,
    /// the output clusters might look like:
    /// \code
    /// I   l i k e   fl o w e r s ,   m u ffi n s   a n d   w a ffl e s .
    /// 0 0 0 0 0 0 0 0  0 1 1 1 1 1 1 1 1 1   2 2 2 2 2 2 2 2 2 3   3 3 3
    /// 0 1 2 3 4 5 6 7  9 0 1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8 9 0   3 4 5
    /// \endcode
    ///
    /// In order to e.g. color the word "muffins", the beginning
    /// and end codepoint indices of the word have to be determined,
    /// in this case 16 and 22. After shaping, any glyphs belonging
    /// to the word "muffins" will have cluster values between and
    /// including 16 and 22. In the example above the clusters
    /// 16, 17, 18, 21 and 22 belong to the word "muffins".
    /// Coloring the glyphs with those indices will result in the
    /// word "muffins" being colored.
    ///
    /// The same applies to "flowers" and "waffles" in the example
    /// above.
    ///
    /// Because merging and decomposition of codepoints cannot
    /// happen beyond word boundaries, applying properties to
    /// glyphs using the above method is safe when segmenting
    /// based on words. As can be seen above it would not work
    /// when attempting to apply a different property to the
    /// single graphemes 'f', 'l' or 'i' since they can be
    /// merged with neighbouring graphemes into a single glyph.
    ///
    /// The opposite, decomposition, of the following input:
    /// \code
    /// I   f i n d   c l i c h é s   f u n n y .
    /// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1
    /// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
    /// \endcode
    /// into glyphs would look like:
    /// \code
    /// I   f i n d   c l i c h e + s   f u n n y .
    /// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1
    /// 0 1 2 3 4 5 6 7 8 9 0 1 2 2 3 4 5 6 7 8 9 0
    /// \endcode
    /// The + at cluster 12 is a placeholder for the acute accent.
    /// This can occur if the font provides the glyph for the accent
    /// seperate from the base glyph e which also has the cluster
    /// value 12. The codepoint é is thus decomposed into a base glyph
    /// and a mark glyph. The cluster value of the mark in such a
    /// decomposition will be identical to the base. Because of this,
    /// the same procedure as demonstrated in the fist example can be
    /// applied here as well.
    ///
    /// The above examples are just simple examples of how to map
    /// input codepoint indices to output cluster values. While
    /// merging and decomposition are an exception in latin script,
    /// they can occur very frequently in other scripts. The mapping
    /// procedure described above will work for all scripts.
    ///
    /// Once the boundaries of the cluster values whose properties to
    /// modify have been determined, they can be used from within
    /// the callable to set said properties on a glyph by glyph basis.
    ///
    /// The callable will be called in the order in which glyph
    /// geometry is generated. This does not always happen in
    /// ascending cluster order such as in right-to-left text where
    /// it happens in descending cluster order.
    ///
    /// Be aware that while changing the character size per glyph
    /// is not possible, changing its style or outline thickness
    /// is. Doing this, however, might lead to slight inconsistencies
    /// when the text bounds are computed at the end of the geometry
    /// update process. The same applies to the italic style.
    ///
    /// In contrast, changing the fill or outline color is safe
    /// since they don't have any effect on the pixel coverage of
    /// the glyph.
    ///
    /// Setting the underlined and strikethrough styles per glyph
    /// is technically possible but not yet implemented.
    ///
    /// Note: Because text bounds are computed based on the
    /// geometry, it is not safe or reliable to query the text bounds
    /// from within this callable. If it is absolutely necessary
    /// to make decisions within this callable based on text bounds,
    /// multiple geometry updates will be necessary. The first
    /// geometry update is run with the pre-processor set to
    /// pass through data. Based on the first update the text bounds
    /// can be queried and stored. The stored text bounds can then
    /// be used in the second geometry update.
    ///
    ////////////////////////////////////////////////////////////
    using GlyphPreProcessor = std::function<
        void(const ShapedGlyph& shapedGlyph, Style& style, Color& fillColor, Color& outlineColor, float& outlineThickness)>;
    // TODO P0: ?

    ////////////////////////////////////////////////////////////
    /// \brief Set the glyph pre-processor to be called per glyph
    ///
    /// The glyph pre-processor is a callable that will be called
    /// with glyph data to be pre-processed.
    ///
    /// \param glyphPreProcessor The glyph pre-processor to be called per glyph, pass an empty pre-processor to disable pre-processing
    ///
    ////////////////////////////////////////////////////////////
    void setGlyphPreProcessor(GlyphPreProcessor glyphPreProcessor);

    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// entity in the entity's coordinate system.
    ///
    /// \return Local bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FloatRect& getLocalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global bounding rectangle of the entity
    ///
    /// The returned rectangle is in global coordinates, which means
    /// that it takes into account the transformations (translation,
    /// rotation, scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// text in the global 2D world's coordinate system.
    ///
    /// \return Global bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] FloatRect getGlobalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Draw the text to a render target
    ///
    /// \param target Render target to draw to
    /// \param states Current render states
    ///
    ////////////////////////////////////////////////////////////
    void draw(RenderTarget& target, RenderStates states) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a read-only span to the text's vertices.
    ///
    /// The vertices are recalculated, if necessary, when this function
    /// is invoked. Therefore it is not thread-safe.
    ///
    /// The returned span fulfills the following properties:
    ///
    /// - The subrange `[0, getFillVerticesStartIndex()]` contains
    ///   all the text outline vertices, if any.
    ///
    /// - The remaining subrange contains all text fill vertices, if any.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] ConstVertexSpan getVertices() const
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data(), m_vertices.size()};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get a mutable span to the text's vertices.
    ///
    /// \see `getVertices`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] VertexSpan getVerticesMut()
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data(), m_vertices.size()};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the index of the first fill vertex.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] base::SizeT getFillVerticesStartIndex() const
    {
        ensureGeometryUpdate(*m_font);
        return m_fillVerticesStartIndex;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] ConstVertexSpan getFillVertices() const
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data() + m_fillVerticesStartIndex, m_vertices.size() - m_fillVerticesStartIndex};
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] ConstVertexSpan getOutlineVertices() const
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data(), m_fillVerticesStartIndex};
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] VertexSpan getFillVerticesMut() const
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data() + m_fillVerticesStartIndex, m_vertices.size() - m_fillVerticesStartIndex};
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] VertexSpan getOutlineVerticesMut() const
    {
        ensureGeometryUpdate(*m_font);
        return {m_vertices.data(), m_fillVerticesStartIndex};
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Make sure the text's geometry is updated
    ///
    /// All the attributes related to rendering are cached, such
    /// that the geometry is only updated when necessary.
    ///
    ////////////////////////////////////////////////////////////
    void ensureGeometryUpdate(const Font& font) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    base::SizeT precomputeQuadCount() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    /* Ordered to minimize padding */
    String                            m_string;       //!< String to display
    mutable base::Vector<Vertex>      m_vertices;     //!< Vertex array containing the outline and fill geometry
    mutable base::Vector<ShapedGlyph> m_glyphs;       //!< Cluster positions
    mutable FloatRect                 m_bounds;       //!< Bounding rectangle of the text (in local coordinates)
    const Font*                       m_font{};       //!< Font used to display the string
    mutable base::SizeT m_fillVerticesStartIndex{};   //!< Index in the vertex array where the fill vertices start
    unsigned int        m_characterSize{30u};         //!< Base size of characters, in pixels
    float               m_letterSpacingFactor{1.f};   //!< Spacing factor between letters
    float               m_lineSpacingFactor{1.f};     //!< Spacing factor between lines
    float               m_outlineThickness{0.f};      //!< Thickness of the text's outline
    Color               m_fillColor{Color::White};    //!< Text fill color
    Color               m_outlineColor{Color::Black}; //!< Text outline color

public:
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;

private:
    Style             m_style{Style::Regular};                       //!< Text style (see Style enum)
    LineAlignment     m_lineAlignment{LineAlignment::Default};       //!< Line alignment for a multi-line text
    TextOrientation   m_textOrientation{TextOrientation::Default};   //!< Text orientation
    ClusterGrouping   m_clusterGrouping{ClusterGrouping::Character}; //!< Cluster grouping algorithm
    GlyphPreProcessor m_glyphPreProcessor;                           //!< Glyph pre-processor
    mutable bool      m_geometryNeedUpdate{};                        //!< Does the geometry need to be recomputed?

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(Font);
};

SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(Text::Style);

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Text
/// \ingroup graphics
///
/// `sf::Text` is a drawable class that allows to easily display
/// some text with custom style and color on a render target.
///
/// It inherits all the functions from `sf::Transformable`:
/// position, rotation, scale, origin. It also adds text-specific
/// properties such as the font to use, the character size,
/// the font style (bold, italic, underlined and strike through), the
/// text color, the outline thickness, the outline color, the character
/// spacing, the line spacing and the text to display of course.
/// It also provides convenience functions to calculate the
/// graphical size of the text, or to get the global position
/// of a given character.
///
/// `sf::Text` works in combination with the `sf::Font` class, which
/// loads and provides the glyphs (visual characters) of a given font.
///
/// The separation of `sf::Font` and `sf::Text` allows more flexibility
/// and better performances: indeed a `sf::Font` is a heavy resource,
/// and any operation on it is slow (often too slow for real-time
/// applications). On the other side, a `sf::Text` is a lightweight
/// object which can combine the glyphs data and metrics of a `sf::Font`
/// to display any text on a render target.
///
/// It is important to note that the `sf::Text` instance doesn't
/// copy the font that it uses, it only keeps a reference to it.
/// Thus, a `sf::Font` must not be destructed while it is
/// used by a `sf::Text` (i.e. never write a function that
/// uses a local `sf::Font` instance for creating a text).
///
/// See also the note on coordinates and undistorted rendering in `sf::Transformable`.
///
/// Usage example:
/// \code
/// // Open a font
/// const auto font = sf::Font::openFromFile("arial.ttf").value();
///
/// // Create a text
/// sf::Text text(font, "hello");
/// text.setCharacterSize(30);
/// text.setStyle(sf::Text::Style::Bold);
/// text.setFillColor(sf::Color::Red);
///
/// // Draw it
/// window.draw(text);
/// \endcode
///
/// \see `sf::Font`, `sf::Transformable`
///
////////////////////////////////////////////////////////////
