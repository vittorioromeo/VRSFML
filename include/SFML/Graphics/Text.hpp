#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/AnchorPointMixin.hpp"
#include "SFML/System/LifetimeDependant.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/EnumClassBitwiseOps.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class RenderTarget;
struct RenderStates;
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
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using Settings = TextData;

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
    /// \param settings       Settings of the text
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Text(const Font& font, const Settings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from a temporary font
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Text(const Font&& font, const Settings& settings) = delete;

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
    [[nodiscard]] Vector2f findCharacterPos(base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, ...) that are applied to the entity.
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
    /// rotation, scale, ...) that are applied to the entity.
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
    [[nodiscard, gnu::always_inline]] base::Span<const Vertex> getVertices() const
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
    [[nodiscard, gnu::always_inline]] base::Span<Vertex> getVerticesMut()
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
        return m_fillVerticesStartIndex;
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
    // Member data
    ////////////////////////////////////////////////////////////
    /* Ordered to minimize padding */
    String                              m_string;     //!< String to display
    mutable base::Vector<Vertex> m_vertices;   //!< Vertex array containing the outline and fill geometry
    mutable FloatRect                   m_bounds;     //!< Bounding rectangle of the text (in local coordinates)
    const Font*                         m_font{};     //!< Font used to display the string
    mutable base::SizeT m_fillVerticesStartIndex{};   //!< Index in the vertex array where the fill vertices start
    unsigned int        m_characterSize{30u};         //!< Base size of characters, in pixels
    float               m_letterSpacing{1.f};         //!< Spacing factor between letters
    float               m_lineSpacing{1.f};           //!< Spacing factor between lines
    float               m_outlineThickness{0.f};      //!< Thickness of the text's outline
    Color               m_fillColor{Color::White};    //!< Text fill color
    Color               m_outlineColor{Color::Black}; //!< Text outline color

public:
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;

private:
    Style        m_style{Style::Regular}; //!< Text style (see Style enum)
    mutable bool m_geometryNeedUpdate{};  //!< Does the geometry need to be recomputed?

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
