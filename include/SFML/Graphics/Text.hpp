#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/TextBase.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/System/LifetimeDependant.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class Texture;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Graphical text that can be drawn to a render target
///
/// Uses legacy `sf::Font` for lazy glyph rasterization.
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Text : public TextBase<Text>
{
public:
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
    /// \param font Font to use
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
    /// \brief Set whether the text is bold
    ///
    /// Bold affects which glyph variant is loaded from the font.
    /// By default, text is not bold.
    ///
    /// \param bold True to enable bold, false to disable
    ///
    /// \see `isBold`
    ///
    ////////////////////////////////////////////////////////////
    void setBold(bool bold);

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
    /// \brief Get whether the text is bold
    ///
    /// \return True if the text is bold
    ///
    /// \see `setBold`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBold() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Font& getFontSource() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Texture& getTexture() const;

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
    /// \brief Get the outline thickness of the text
    ///
    /// \return Outline thickness of the text, in pixels
    ///
    /// \see `setOutlineThickness`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getOutlineThickness() const;

private:
    ////////////////////////////////////////////////////////////
    friend class TextBase<Text>;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const Font*  m_font{};                //!< Font used to display the string
    unsigned int m_characterSize{30u};    //!< Base size of characters, in pixels
    float        m_outlineThickness{0.f}; //!< Thickness of the text's outline
    bool         m_bold{false};           //!< Bold characters

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(Font);
};

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
/// text.setBold(true);
/// text.setFillColor(sf::Color::Red);
///
/// // Draw it
/// window.draw(text);
/// \endcode
///
/// \see `sf::Font`, `sf::Transformable`
///
////////////////////////////////////////////////////////////
