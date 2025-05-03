#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Base/ClampMacro.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Remainder.hpp"


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
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color whiteMask(const base::U8 alpha)
    {
        return {255u, 255u, 255u, alpha};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Construct a fully black color with a given alpha
    ///
    /// \param alpha Alpha channel of the black mask
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color blackMask(const base::U8 alpha)
    {
        return {0u, 0u, 0u, alpha};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct the color from 32-bit unsigned integer
    ///
    /// \param color Number containing the RGBA components (in that order)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color fromRGBA(const base::U32 color)
    {
        return {static_cast<base::U8>((color & 0xff'00'00'00) >> 24),
                static_cast<base::U8>((color & 0x00'ff'00'00) >> 16),
                static_cast<base::U8>((color & 0x00'00'ff'00) >> 8),
                static_cast<base::U8>((color & 0x00'00'00'ff) >> 0)};
    }


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
    [[nodiscard, gnu::always_inline, gnu::const]] static constexpr Color fromHSLA(HSL hsl, const base::U8 alpha = 255u)
    {
        auto& [hue, saturation, lightness] = hsl;

        hue = base::positiveRemainder(hue, 360.f);

        const auto clampBetweenZeroAndOne = [](float value) -> float
        { return value < 0.f ? 0.f : (value > 1.f ? 1.f : value); };

        saturation = clampBetweenZeroAndOne(saturation);
        lightness  = clampBetweenZeroAndOne(lightness);

        // `maxChroma` and `minChroma` define the range for each color component
        // `maxChroma` is the upper bound, `minChroma` is the lower bound
        const float maxChroma = lightness < 0.5f ? lightness * (1 + saturation)
                                                 : lightness + saturation - lightness * saturation;
        const float minChroma = 2 * lightness - maxChroma;

        const auto hueToRGB = [&](float normalizedHue) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN -> float
        {
            if (normalizedHue < 0.f)
                normalizedHue += 1.f;

            if (normalizedHue > 1.f)
                normalizedHue -= 1.f;

            if (normalizedHue < 1.f / 6.f)
                return minChroma + (maxChroma - minChroma) * 6.f * normalizedHue;

            if (normalizedHue < 1.f / 2.f)
                return maxChroma;

            if (normalizedHue < 2.f / 3.f)
                return minChroma + (maxChroma - minChroma) * (2.f / 3.f - normalizedHue) * 6.f;

            return minChroma;
        };

        const float normalizedHue = hue / 360.f;

        // NOLINTBEGIN(bugprone-incorrect-roundings)
        return {static_cast<base::U8>(hueToRGB(normalizedHue + 1.f / 3.f) * 255.f + 0.5f),
                static_cast<base::U8>(hueToRGB(normalizedHue) * 255 + 0.5f),
                static_cast<base::U8>(hueToRGB(normalizedHue - 1.f / 3.f) * 255.f + 0.5f),
                alpha};
        // NOLINTEND(bugprone-incorrect-roundings)
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr HSL toHSL() const
    {
        const float rNorm = static_cast<float>(r) / 255.f;
        const float gNorm = static_cast<float>(g) / 255.f;
        const float bNorm = static_cast<float>(b) / 255.f;

        const float max    = SFML_BASE_MAX(SFML_BASE_MAX(rNorm, gNorm), bNorm);
        const float min    = SFML_BASE_MIN(SFML_BASE_MIN(rNorm, gNorm), bNorm);
        const float chroma = max - min;

        float hue = 0.f;
        if (chroma != 0.f)
        {
            if (max == rNorm)
                hue = (gNorm - bNorm) / chroma;
            else if (max == gNorm)
                hue = (bNorm - rNorm) / chroma + 2.f;
            else
                hue = (rNorm - gNorm) / chroma + 4.f;

            hue *= 60.f;

            if (hue < 0.f)
                hue += 360.f;
        }

        const float lightness = (max + min) / 2.f;

        float saturation = 0.f;
        if (chroma != 0.f)
        {
            if (lightness == 0.f || lightness == 1.f)
                saturation = 0.f;
            else
                saturation = chroma / (1.f - SFML_BASE_MATH_FABSF(2.f * lightness - 1.f));
        }

        return {hue, saturation, lightness};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Retrieve the color as a 32-bit unsigned integer
    ///
    /// \return Color represented as a 32-bit unsigned integer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr base::U32 toInteger() const
    {
        return static_cast<base::U32>((r << 24) | (g << 16) | (b << 8) | a);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Create a new color from the current one with the alpha component changed
    ///
    /// \param alpha Alpha component (0 to 255)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withAlpha(const base::U8 alpha) const
    {
        return {r, g, b, alpha};
    }


    // TODO P1: withRed, withGreen, withBlue


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Color withHueMod(const float hueMod) const
    {
        auto hsl = toHSL();
        hsl.hue  = base::positiveRemainder(hsl.hue + hueMod, 360.f);
        return Color::fromHSLA(hsl, a);
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    constexpr Color withSaturation(const float saturation) const
    {
        auto hsl       = toHSL();
        hsl.saturation = SFML_BASE_CLAMP(saturation, 0.f, 1.f);
        return fromHSLA(hsl, a);
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    constexpr Color withLightness(const float lightness) const
    {
        auto hsl      = toHSL();
        hsl.lightness = SFML_BASE_CLAMP(lightness, 0.f, 1.f);
        return fromHSLA(hsl, a);
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TVec4>
    static constexpr Color fromVec4(const TVec4& vec)
    {
        const auto convert = []<typename T>(const T value)
        { return static_cast<base::U8>(SFML_BASE_CLAMP(value * T{255}, T{0}, T{255})); };

        return {convert(vec.x), convert(vec.y), convert(vec.z), convert(vec.w)};
    }


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
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr TVec4 toVec4() const
    {
        using DimType = decltype(TVec4{}.x);

        return {static_cast<DimType>(r) / DimType{255},
                static_cast<DimType>(g) / DimType{255},
                static_cast<DimType>(b) / DimType{255},
                static_cast<DimType>(a) / DimType{255}};
    }


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
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator+(const Color lhs, const Color rhs)
{
    const auto clampedAdd = [](base::U8 l, base::U8 r) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN -> base::U8
    {
        const int intResult = int{l} + int{r};
        return static_cast<base::U8>(intResult < 255 ? intResult : 255);
    };

    return {clampedAdd(lhs.r, rhs.r), clampedAdd(lhs.g, rhs.g), clampedAdd(lhs.b, rhs.b), clampedAdd(lhs.a, rhs.a)};
}


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
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator-(const Color lhs, const Color rhs)
{
    const auto clampedSub = [](base::U8 l, base::U8 r) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN -> base::U8
    {
        const int intResult = int{l} - int{r};
        return static_cast<base::U8>(intResult > 0 ? intResult : 0);
    };

    return {clampedSub(lhs.r, rhs.r), clampedSub(lhs.g, rhs.g), clampedSub(lhs.b, rhs.b), clampedSub(lhs.a, rhs.a)};
}


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
[[nodiscard, gnu::always_inline, gnu::const]] constexpr Color operator*(const Color lhs, const Color rhs)
{
    const auto scaledMul = [](base::U8 l, base::U8 r) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN -> base::U8
    {
        const auto uint16Result = static_cast<base::U16>(base::U16{l} * base::U16{r});
        return static_cast<base::U8>(uint16Result / 255u);
    };

    return {scaledMul(lhs.r, rhs.r), scaledMul(lhs.g, rhs.g), scaledMul(lhs.b, rhs.b), scaledMul(lhs.a, rhs.a)};
}


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
[[gnu::always_inline]] constexpr Color& operator+=(Color& lhs, const Color rhs)
{
    return lhs = lhs + rhs;
}


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
[[gnu::always_inline]] constexpr Color& operator-=(Color& lhs, const Color rhs)
{
    return lhs = lhs - rhs;
}


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
[[gnu::always_inline]] constexpr Color& operator*=(Color& lhs, const Color rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Color Color::Black(0, 0, 0);
inline constexpr Color Color::White(255, 255, 255);
inline constexpr Color Color::Red(255, 0, 0);
inline constexpr Color Color::Green(0, 255, 0);
inline constexpr Color Color::Blue(0, 0, 255);
inline constexpr Color Color::Yellow(255, 255, 0);
inline constexpr Color Color::Magenta(255, 0, 255);
inline constexpr Color Color::Cyan(0, 255, 255);
inline constexpr Color Color::Transparent(0, 0, 0, 0);

inline constexpr Color Color::Brown{128u, 80u, 32u};
inline constexpr Color Color::Orange{255u, 128u, 0u};
inline constexpr Color Color::Pink{255u, 128u, 192u};
inline constexpr Color Color::BabyPink{255u, 192u, 224u};
inline constexpr Color Color::HotPink{255u, 0u, 192u};
inline constexpr Color Color::Salmon{255u, 128u, 128u};
inline constexpr Color Color::Violet{128u, 0u, 255u};
inline constexpr Color Color::Purple{64u, 0u, 128u};
inline constexpr Color Color::Peach{255u, 128u, 96u};
inline constexpr Color Color::Lime{128u, 255u, 0u};
inline constexpr Color Color::Mint{64u, 255u, 192u};
inline constexpr Color Color::Gray{128u, 128u, 128u};

inline constexpr Color Color::LightBlack{64u, 64u, 64u};
inline constexpr Color Color::LightBlue{128u, 128u, 255u};
inline constexpr Color Color::LightRed{255u, 128u, 128u};
inline constexpr Color Color::LightMagenta{255u, 128u, 255u};
inline constexpr Color Color::LightGreen{128u, 255u, 128u};
inline constexpr Color Color::LightCyan{128u, 255u, 255u};
inline constexpr Color Color::LightYellow{255u, 255u, 128u};
inline constexpr Color Color::LightBrown{192u, 128u, 64u};
inline constexpr Color Color::LightOrange{255u, 160u, 64u};
inline constexpr Color Color::LightPink{255u, 160u, 224u};
inline constexpr Color Color::LightBabyPink{255u, 208u, 232u};
inline constexpr Color Color::LightHotPink{255u, 96u, 224u};
inline constexpr Color Color::LightSalmon{255u, 160u, 160u};
inline constexpr Color Color::LightViolet{160u, 96u, 255u};
inline constexpr Color Color::LightPurple{128u, 64u, 192u};
inline constexpr Color Color::LightPeach{255u, 160u, 128u};
inline constexpr Color Color::LightLime{192u, 255u, 128u};
inline constexpr Color Color::LightMint{128u, 255u, 22u};
inline constexpr Color Color::LightGray{192u, 192u, 192u};

inline constexpr Color Color::DarkBlue{0u, 0u, 128u};
inline constexpr Color Color::DarkRed{128u, 0u, 0u};
inline constexpr Color Color::DarkMagenta{128u, 0u, 128u};
inline constexpr Color Color::DarkGreen{0u, 128u, 0u};
inline constexpr Color Color::DarkCyan{0u, 128u, 128u};
inline constexpr Color Color::DarkYellow{128u, 128u, 0u};
inline constexpr Color Color::DarkWhite{128u, 128u, 128u};
inline constexpr Color Color::DarkBrown{64u, 32u, 0u};
inline constexpr Color Color::DarkOrange{128u, 64u, 0u};
inline constexpr Color Color::DarkPink{128u, 64u, 96u};
inline constexpr Color Color::DarkBabyPink{160u, 96u, 128u};
inline constexpr Color Color::DarkHotPink{128u, 0u, 96u};
inline constexpr Color Color::DarkSalmon{128u, 64u, 64u};
inline constexpr Color Color::DarkViolet{64u, 0u, 128u};
inline constexpr Color Color::DarkPurple{32u, 0u, 64u};
inline constexpr Color Color::DarkPeach{128u, 64u, 48u};
inline constexpr Color Color::DarkLime{64u, 128u, 0u};
inline constexpr Color Color::DarkMint{32u, 128u, 96u};
inline constexpr Color Color::DarkGray{64u, 64u, 64u};

inline constexpr Color Color::VeryDarkBlue{0u, 0u, 64u};
inline constexpr Color Color::VeryDarkRed{64u, 0u, 0u};
inline constexpr Color Color::VeryDarkMagenta{64u, 0u, 64u};
inline constexpr Color Color::VeryDarkGreen{0u, 64u, 0u};
inline constexpr Color Color::VeryDarkCyan{0u, 64u, 64u};
inline constexpr Color Color::VeryDarkYellow{64u, 64u, 0u};
inline constexpr Color Color::VeryDarkWhite{64u, 64u, 64u};
inline constexpr Color Color::VeryDarkBrown{32u, 16u, 0u};
inline constexpr Color Color::VeryDarkOrange{64u, 32u, 0u};
inline constexpr Color Color::VeryDarkPink{64u, 32u, 48u};
inline constexpr Color Color::VeryDarkBabyPink{80u, 48u, 64u};
inline constexpr Color Color::VeryDarkHotPink{64u, 0u, 48u};
inline constexpr Color Color::VeryDarkSalmon{64u, 32u, 32u};
inline constexpr Color Color::VeryDarkViolet{32u, 0u, 64u};
inline constexpr Color Color::VeryDarkPurple{16u, 0u, 32u};
inline constexpr Color Color::VeryDarkPeach{64u, 32u, 24u};
inline constexpr Color Color::VeryDarkLime{32u, 64u, 0u};
inline constexpr Color Color::VeryDarkMint{16u, 64u, 48u};
inline constexpr Color Color::VeryDarkGray{32u, 32u, 32u};

} // namespace sf


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
