#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class RenderTarget;
struct RenderStates;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief CRTP base class for text objects
///
/// Holds all data and methods shared between `Text` (legacy
/// lazy-loading path) and `GlyphMappedText` (explicit preloading
/// path).
///
/// Derived classes must provide:
/// - `const auto& getFontSource() const` -- returns Font& or GlyphMapping&
/// - `unsigned int getCharacterSize() const`
/// - `float getOutlineThickness() const`
/// - `const Texture& getTexture() const`
/// - `bool isBold() const`
///
////////////////////////////////////////////////////////////
template <typename TDerived>
class SFML_GRAPHICS_API TextBase :
    public TransformableMixin<TDerived>,
    public GlobalAnchorPointMixin<TDerived>,
    public LocalAnchorPointMixin<TDerived>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Set the text's string
    ///
    /// The `string` argument is a `sf::UnicodeString`, which can
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
    void setString(const UnicodeString& string);

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
    /// \brief Set whether the text is italic
    ///
    /// \param italic True to enable italic, false to disable
    ///
    /// \see `isItalic`
    ///
    ////////////////////////////////////////////////////////////
    void setItalic(bool italic);

    ////////////////////////////////////////////////////////////
    /// \brief Set whether the text is underlined
    ///
    /// \param underlined True to enable underline, false to disable
    ///
    /// \see `isUnderlined`
    ///
    ////////////////////////////////////////////////////////////
    void setUnderlined(bool underlined);

    ////////////////////////////////////////////////////////////
    /// \brief Set whether the text has a strike through
    ///
    /// \param strikeThrough True to enable strike through, false to disable
    ///
    /// \see `isStrikeThrough`
    ///
    ////////////////////////////////////////////////////////////
    void setStrikeThrough(bool strikeThrough);

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
    /// \brief Get the text's string
    ///
    /// The returned string is a `sf::UnicodeString`, which can automatically
    /// be converted to standard string types. So, the following
    /// lines of code are all valid:
    /// \code
    /// sf::UnicodeString s1 = text.getString();
    /// std::string       s2 = text.getString();
    /// std::wstring      s3 = text.getString();
    /// \endcode
    ///
    /// \return Text's string
    ///
    /// \see `setString`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const UnicodeString& getString() const;

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
    /// \brief Get whether the text is italic
    ///
    /// \return True if the text is italic
    ///
    /// \see `setItalic`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isItalic() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get whether the text is underlined
    ///
    /// \return True if the text is underlined
    ///
    /// \see `setUnderlined`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isUnderlined() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get whether the text has a strike through
    ///
    /// \return True if the text has a strike through
    ///
    /// \see `setStrikeThrough`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isStrikeThrough() const;

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

    ////////////////////////////////////////////////////////////
    /// \brief Return the position of the `index`-th character
    ///
    /// This function computes the visual position of a character
    /// from its index in the string. The returned position is
    /// in global coordinates (translation, rotation, scale and
    /// origin are applied).
    /// If `index` is out of range, the position of the end of
    /// the string is returned.
    ///
    /// \param index Index of the character
    ///
    /// \return Position of the character
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f findCharacterPos(base::SizeT index) const;


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
    [[nodiscard]] const Rect2f& getLocalBounds() const;

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
    [[nodiscard]] Rect2f getGlobalBounds() const;

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
        ensureGeometryUpdate();
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
        ensureGeometryUpdate();
        return {m_vertices.data(), m_vertices.size()};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the index where the fill vertices start in `getVertices()`
    ///
    /// The vertex array returned by `getVertices` is laid out as
    /// `[outline vertices ... fill vertices]`. This function
    /// returns the index of the first fill vertex, which is also
    /// the count of outline vertices.
    ///
    /// \return Start index of the fill vertex sub-range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] base::SizeT getFillVerticesStartIndex() const
    {
        return m_fillVerticesStartIndex;
    }

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Construct from common text data fields
    ///
    ////////////////////////////////////////////////////////////
    template <typename TData>
    explicit TextBase(const TData& data);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~TextBase() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Copy operations
    ///
    ////////////////////////////////////////////////////////////
    TextBase(const TextBase&)            = default;
    TextBase& operator=(const TextBase&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Move operations
    ///
    ////////////////////////////////////////////////////////////
    TextBase(TextBase&&) noexcept            = default;
    TextBase& operator=(TextBase&&) noexcept = default;

    ////////////////////////////////////////////////////////////
    /// \brief Make sure the text's geometry is updated
    ///
    /// All the attributes related to rendering are cached, such
    /// that the geometry is only updated when necessary.
    ///
    ////////////////////////////////////////////////////////////
    void ensureGeometryUpdate() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    UnicodeString m_string; //!< String to display

    mutable base::Vector<Vertex> m_vertices;        //!< Vertex array containing the outline and fill geometry
    mutable Rect2f               m_bounds;          //!< Bounding rectangle of the text (in local coordinates)
    mutable base::SizeT m_fillVerticesStartIndex{}; //!< Index in the vertex array where the fill vertices start

    float m_letterSpacing{1.f};         //!< Spacing factor between letters
    float m_lineSpacing{1.f};           //!< Spacing factor between lines
    Color m_fillColor{Color::White};    //!< Text fill color
    Color m_outlineColor{Color::Black}; //!< Text outline color

public:
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;

protected:
    bool         m_italic{false};        //!< Italic characters
    bool         m_underlined{false};    //!< Underlined characters
    bool         m_strikeThrough{false}; //!< Strike through characters
    mutable bool m_geometryNeedUpdate{}; //!< Does the geometry need to be recomputed?
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TextBase
/// \ingroup graphics
///
/// `sf::TextBase` is the **CRTP** base shared by `sf::Text`
/// and `sf::GlyphMappedText`. It implements the parts of the
/// text rendering pipeline that do not depend on the source of
/// the glyphs:
///
/// \li the string itself,
/// \li letter spacing, line spacing, italic/underline/strike,
/// \li fill and outline colors (with separate alpha setters),
/// \li the cached vertex buffer that backs the rendered text,
/// \li the public transformable members,
/// \li bounds queries (`getLocalBounds`, `getGlobalBounds`),
/// \li per-character position lookup (`findCharacterPos`).
///
/// Concrete subclasses provide:
/// \li `getFontSource()` -- returns either a `sf::Font&` (legacy
///     lazy-loading path) or a `sf::GlyphMapping&` (precomputed
///     path).
/// \li `getCharacterSize()`, `getOutlineThickness()`, `isBold()`
/// \li `getTexture()` -- the glyph atlas texture.
///
/// `TextBase` is not meant to be used directly. Use `sf::Text`
/// or `sf::GlyphMappedText`, depending on whether your text is
/// dynamic or precomputed.
///
/// \see `sf::Text`, `sf::GlyphMappedText`, `sf::TextData`
///
////////////////////////////////////////////////////////////
