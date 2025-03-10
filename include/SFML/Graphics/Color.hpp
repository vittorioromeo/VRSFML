#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility class for manipulating RGBA colors
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API Color
{
    ////////////////////////////////////////////////////////////
    /// \brief Construct a fully white color with a given alpha
    ///
    /// \param alpha Alpha channel of the white mask
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color whiteMask(base::U8 alpha);

    ////////////////////////////////////////////////////////////
    /// \brief Construct a fully black color with a given alpha
    ///
    /// \param alpha Alpha channel of the black mask
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color blackMask(base::U8 alpha);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the color from 32-bit unsigned integer
    ///
    /// \param color Number containing the RGBA components (in that order)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color fromRGBA(base::U32 color);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct HSL
    {
        float hue;        //!< Hue in degrees (between `[0, 360)`)
        float saturation; //!< Saturation (between `[0, 1]`)
        float lightness;  //!< Lightness (between `[0, 1]`)
    };

    ////////////////////////////////////////////////////////////
    /// \brief Construct the color from HSLA components
    ///
    /// \param hue        Hue component (angle in degrees)
    /// \param saturation Saturation component (from 0 to 1)
    /// \param lightness  Lightness component (from 0 to 1)
    /// \param alpha      Alpha component (0 to 255)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color fromHSLA(HSL hsl, base::U8 alpha = 255u);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr HSL toHSL() const;

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve the color as a 32-bit unsigned integer
    ///
    /// \return Color represented as a 32-bit unsigned integer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::U32 toInteger() const;

    ////////////////////////////////////////////////////////////
    /// \brief Create a new color from the current one with the alpha component changed
    ///
    /// \param alpha Alpha component (0 to 255)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withAlpha(base::U8 alpha) const;
    // TODO P1: withRed, withGreen, withBlue

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withHueMod(float hueMod) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withSaturation(float saturation) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withLightness(float lightness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of the `operator==`
    ///
    /// This operator compares two colors and check if they are equal.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if colors are equal, `false` if they are different
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] constexpr bool operator==(const Color& rhs) const = default;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs, range [0, 1], xyzw
    ///
    ////////////////////////////////////////////////////////////
    template <typename TVec4>
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color fromVec4(const TVec4& vec);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs, range [0, 1], xyzw
    ///
    ////////////////////////////////////////////////////////////
    template <typename TVec4>
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr TVec4 toVec4() const;

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTBEGIN(readability-identifier-naming)
    static const Color Black;       //!< Black predefined color
    static const Color White;       //!< White predefined color
    static const Color Red;         //!< Red predefined color
    static const Color Green;       //!< Green predefined color
    static const Color Blue;        //!< Blue predefined color
    static const Color Yellow;      //!< Yellow predefined color
    static const Color Magenta;     //!< Magenta predefined color
    static const Color Cyan;        //!< Cyan predefined color
    static const Color Transparent; //!< Transparent (black) predefined color

    static const Color Brown;    //!< Brown predefined color
    static const Color Orange;   //!< Orange predefined color
    static const Color Pink;     //!< Pink predefined color
    static const Color BabyPink; //!< Baby pink predefined color
    static const Color HotPink;  //!< Hot pink predefined color
    static const Color Salmon;   //!< Salmon predefined color
    static const Color Violet;   //!< Violet predefined color
    static const Color Purple;   //!< Purple predefined color
    static const Color Peach;    //!< Peach predefined color
    static const Color Lime;     //!< Lime predefined color
    static const Color Mint;     //!< Mint predefined color
    static const Color Gray;     //!< Gray predefined color

    static const Color LightBlack;    //!< Light black predefined color
    static const Color LightBlue;     //!< Light blue predefined color
    static const Color LightRed;      //!< Light red predefined color
    static const Color LightMagenta;  //!< Light magenta predefined color
    static const Color LightGreen;    //!< Light green predefined color
    static const Color LightCyan;     //!< Light cyan predefined color
    static const Color LightYellow;   //!< Light yellow predefined color
    static const Color LightBrown;    //!< Light brown predefined color
    static const Color LightOrange;   //!< Light orange predefined color
    static const Color LightPink;     //!< Light pink predefined color
    static const Color LightBabyPink; //!< Light baby pink predefined color
    static const Color LightHotPink;  //!< Light hot pink predefined color
    static const Color LightSalmon;   //!< Light salmon predefined color
    static const Color LightViolet;   //!< Light violet predefined color
    static const Color LightPurple;   //!< Light purple predefined color
    static const Color LightPeach;    //!< Light peach predefined color
    static const Color LightLime;     //!< Light lime predefined color
    static const Color LightMint;     //!< Light mint predefined color
    static const Color LightGray;     //!< Light gray predefined color

    static const Color DarkBlue;     //!< Dark blue predefined color
    static const Color DarkRed;      //!< Dark red predefined color
    static const Color DarkMagenta;  //!< Dark magenta predefined color
    static const Color DarkGreen;    //!< Dark green predefined color
    static const Color DarkCyan;     //!< Dark cyan predefined color
    static const Color DarkYellow;   //!< Dark yellow predefined color
    static const Color DarkWhite;    //!< Dark white predefined color
    static const Color DarkBrown;    //!< Dark brown predefined color
    static const Color DarkOrange;   //!< Dark orange predefined color
    static const Color DarkPink;     //!< Dark pink predefined color
    static const Color DarkBabyPink; //!< Dark baby pink predefined color
    static const Color DarkHotPink;  //!< Dark hot pink predefined color
    static const Color DarkSalmon;   //!< Dark salmon predefined color
    static const Color DarkViolet;   //!< Dark violet predefined color
    static const Color DarkPurple;   //!< Dark purple predefined color
    static const Color DarkPeach;    //!< Dark peach predefined color
    static const Color DarkLime;     //!< Dark lime predefined color
    static const Color DarkMint;     //!< Dark mint predefined color
    static const Color DarkGray;     //!< Dark gray predefined color

    static const Color VeryDarkBlue;     //!< Very dark blue predefined color
    static const Color VeryDarkRed;      //!< Very dark red predefined color
    static const Color VeryDarkMagenta;  //!< Very dark magenta predefined color
    static const Color VeryDarkGreen;    //!< Very dark green predefined color
    static const Color VeryDarkCyan;     //!< Very dark cyan predefined color
    static const Color VeryDarkYellow;   //!< Very dark yellow predefined color
    static const Color VeryDarkWhite;    //!< Very dark white predefined color
    static const Color VeryDarkBrown;    //!< Very dark brown predefined color
    static const Color VeryDarkOrange;   //!< Very dark orange predefined color
    static const Color VeryDarkPink;     //!< Very dark pink predefined color
    static const Color VeryDarkBabyPink; //!< Very dark babyPink predefined color
    static const Color VeryDarkHotPink;  //!< Very dark hotPink predefined color
    static const Color VeryDarkSalmon;   //!< Very dark salmon predefined color
    static const Color VeryDarkViolet;   //!< Very dark violet predefined color
    static const Color VeryDarkPurple;   //!< Very dark purple predefined color
    static const Color VeryDarkPeach;    //!< Very dark peach predefined color
    static const Color VeryDarkLime;     //!< Very dark lime predefined color
    static const Color VeryDarkMint;     //!< Very dark mint predefined color
    static const Color VeryDarkGray;     //!< Very dark gray predefined color
    // NOLINTEND(readability-identifier-naming)

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::U8 r{};     //!< Red component
    base::U8 g{};     //!< Green component
    base::U8 b{};     //!< Blue component
    base::U8 a{255u}; //!< Alpha (opacity) component
};

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator+`
///
/// This operator returns the component-wise sum of two colors.
/// Components that exceed 255 are clamped to 255.
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Result of \a lhs + \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator+(Color lhs, Color rhs);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator-`
///
/// This operator returns the component-wise subtraction of two colors.
/// Components below 0 are clamped to 0.
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Result of \a lhs - \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator-(Color lhs, Color rhs);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator*`
///
/// This operator returns the component-wise multiplication
/// (also called "modulation") of two colors.
/// Components are then divided by 255 so that the result is
/// still in the range [0, 255].
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Result of \a lhs * \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator*(Color lhs, Color rhs);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator+=`
///
/// This operator computes the component-wise sum of two colors,
/// and assigns the result to the lhs operand.
/// Components that exceed 255 are clamped to 255.
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Reference to \a lhs
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Color& operator+=(Color& lhs, Color rhs);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator-=`
///
/// This operator computes the component-wise subtraction of two colors,
/// and assigns the result to the lhs operand.
/// Components below 0 are clamped to 0.
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Reference to \a lhs
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Color& operator-=(Color& lhs, Color rhs);

////////////////////////////////////////////////////////////
/// \relates Color
/// \brief Overload of the binary `operator*=`
///
/// This operator returns the component-wise multiplication
/// (also called "modulation") of two colors, and assigns
/// the result to the lhs operand.
/// Components are then divided by 255 so that the result is
/// still in the range [0, 255].
///
/// \param lhs  Left operand
/// \param rhs Right operand
///
/// \return Reference to \a lhs
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr Color& operator*=(Color& lhs, Color rhs);

} // namespace sf

#include "SFML/Graphics/Color.inl"


////////////////////////////////////////////////////////////
/// \class sf::Color
/// \ingroup graphics
///
/// `sf::Color` is a simple color class composed of 4 components:
/// \li Red
/// \li Green
/// \li Blue
/// \li Alpha (opacity)
///
/// Each component is a public member, an unsigned integer in
/// the range [0, 255]. Thus, colors can be constructed and
/// manipulated very easily:
///
/// \code
/// sf::Color color(255, 0, 0); // red
/// color.r = 0;                // make it black
/// color.b = 128;              // make it dark blue
/// \endcode
///
/// The fourth component of colors, named "alpha", represents
/// the opacity of the color. A color with an alpha value of
/// 255 will be fully opaque, while an alpha value of 0 will
/// make a color fully transparent, whatever the value of the
/// other components is.
///
/// The most common colors are already defined as static variables:
/// \code
/// auto black       = sf::Color::Black;
/// auto white       = sf::Color::White;
/// auto red         = sf::Color::Red;
/// auto green       = sf::Color::Green;
/// auto blue        = sf::Color::Blue;
/// auto yellow      = sf::Color::Yellow;
/// auto magenta     = sf::Color::Magenta;
/// auto cyan        = sf::Color::Cyan;
/// auto transparent = sf::Color::Transparent;
/// \endcode
///
/// Colors can also be added and modulated (multiplied) using the
/// overloaded operators + and *.
///
////////////////////////////////////////////////////////////
