#include "GraphicsUtil.hpp"
#include "SystemUtil.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Rect2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"


namespace
{
////////////////////////////////////////////////////////////
doctest::String floatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<doctest::String::size_type>(end - buf);
    return {buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
doctest::String intToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<doctest::String::size_type>(end - buf);
    return {buf, len};
}


////////////////////////////////////////////////////////////
doctest::String hexToString(const sf::base::U32 value)
{
    char buf[10];
    buf[0]    = '0';
    buf[1]    = 'x';
    int shift = 32;
    int pos   = 2;
    do
    {
        shift -= 4;
        const unsigned nibble = (value >> shift) & 0xFu;
        buf[pos++]            = static_cast<char>(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
    } while (shift > 0);
    return {buf, static_cast<doctest::String::size_type>(pos)};
}

} // namespace


namespace doctest
{
////////////////////////////////////////////////////////////
String StringMaker<sf::BlendMode>::convert(const sf::BlendMode& blendMode)
{
    return String("( ") + intToString(static_cast<int>(blendMode.colorSrcFactor)) + ", " +
           intToString(static_cast<int>(blendMode.colorDstFactor)) + ", " +
           intToString(static_cast<int>(blendMode.colorEquation)) + ", " +
           intToString(static_cast<int>(blendMode.alphaSrcFactor)) + ", " +
           intToString(static_cast<int>(blendMode.alphaDstFactor)) + ", " +
           intToString(static_cast<int>(blendMode.alphaEquation)) + " )";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::StencilComparison>::convert(const sf::StencilComparison comparison)
{
    switch (comparison)
    {
        case sf::StencilComparison::Never:
            return "Never";
        case sf::StencilComparison::Less:
            return "Less";
        case sf::StencilComparison::LessEqual:
            return "LessEqual";
        case sf::StencilComparison::Greater:
            return "Greater";
        case sf::StencilComparison::GreaterEqual:
            return "GreaterEqual";
        case sf::StencilComparison::Equal:
            return "Equal";
        case sf::StencilComparison::NotEqual:
            return "NotEqual";
        case sf::StencilComparison::Always:
            return "Always";
    }
    return "";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::StencilUpdateOperation>::convert(const sf::StencilUpdateOperation updateOperation)
{
    switch (updateOperation)
    {
        case sf::StencilUpdateOperation::Keep:
            return "Keep";
        case sf::StencilUpdateOperation::Zero:
            return "Zero";
        case sf::StencilUpdateOperation::Replace:
            return "Replace";
        case sf::StencilUpdateOperation::Increment:
            return "Increment";
        case sf::StencilUpdateOperation::Decrement:
            return "Decrement";
        case sf::StencilUpdateOperation::Invert:
            return "Invert";
    }
    return "";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::StencilMode>::convert(const sf::StencilMode& stencilMode)
{
    return String("( ") + StringMaker<sf::StencilComparison>::convert(stencilMode.stencilComparison) + ", " +
           StringMaker<sf::StencilUpdateOperation>::convert(stencilMode.stencilUpdateOperation) + ", " +
           (stencilMode.stencilOnly ? "true" : "false") + ", " +
           intToString(static_cast<unsigned int>(stencilMode.stencilReference.value)) + ", " +
           intToString(static_cast<unsigned int>(stencilMode.stencilMask.value)) + " )";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::Color>::convert(const sf::Color color)
{
    return hexToString(color.toInteger()) + " (r=" + intToString(int{color.r}) + ", g=" + intToString(int{color.g}) +
           ", b=" + intToString(int{color.b}) + ", a=" + intToString(int{color.a}) + ")";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::Transform>::convert(const sf::Transform& transform)
{
    // clang-format off
    float matrix[]{{},  {},  0.f, 0.f,
                   {},  {},  0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   {},  {},  0.f, 1.f};
    // clang-format on

    transform.writeTo4x4Matrix(matrix);

    return floatToString(matrix[0]) + ", " + floatToString(matrix[4]) + ", " + floatToString(matrix[12]) + ", " +
           floatToString(matrix[1]) + ", " + floatToString(matrix[5]) + ", " + floatToString(matrix[13]) + ", " +
           floatToString(matrix[3]) + ", " + floatToString(matrix[7]) + ", " + floatToString(matrix[15]);
}


////////////////////////////////////////////////////////////
namespace
{
doctest::String vec2ToString(const sf::Vec2<float> v)
{
    return doctest::String("(") + floatToString(v.x) + ", " + floatToString(v.y) + ")";
}

template <typename T>
doctest::String rectToString(const sf::Rect2<T>& rect)
{
    if constexpr (sf::base::isFloatingPoint<T>)
        return doctest::String("(position=(") + floatToString(rect.position.x) + ", " + floatToString(rect.position.y) +
               "), size=(" + floatToString(rect.size.x) + ", " + floatToString(rect.size.y) + "))";
    else
        return doctest::String("(position=(") + intToString(rect.position.x) + ", " + intToString(rect.position.y) +
               "), size=(" + intToString(rect.size.x) + ", " + intToString(rect.size.y) + "))";
}
} // namespace


////////////////////////////////////////////////////////////
String StringMaker<sf::View>::convert(const sf::View& view)
{
    return String("( center=") + vec2ToString(view.center) + ", size=" + vec2ToString(view.size) +
           ", rotation=" + floatToString(view.rotation.asDegrees()) + " deg" +
           ", viewport=" + rectToString(view.viewport) + ", scissor=" + rectToString(view.scissor) + " )";
}


////////////////////////////////////////////////////////////
template <typename T>
String StringMaker<sf::Rect2<T>>::convert(const sf::Rect2<T>& rect)
{
    return rectToString(rect);
}


////////////////////////////////////////////////////////////
String StringMaker<sf::View::ScissorRect>::convert(const sf::View::ScissorRect& scissorRect)
{
    return rectToString(static_cast<const sf::Rect2<float>&>(scissorRect));
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the rect types actually used by tests.
template struct StringMaker<sf::Rect2<int>>;
template struct StringMaker<sf::Rect2<float>>;

} // namespace doctest


////////////////////////////////////////////////////////////
bool operator==(const sf::Transform& lhs, const Approx<sf::Transform>& rhs)
{
    // clang-format off
    float lhsMatrix[]{{},  {},  0.f, 0.f,
                      {},  {},  0.f, 0.f,
                      0.f, 0.f, 1.f, 0.f,
                      {},  {},  0.f, 1.f};

    float rhsMatrix[]{{},  {},  0.f, 0.f,
                      {},  {},  0.f, 0.f,
                      0.f, 0.f, 1.f, 0.f,
                      {},  {},  0.f, 1.f};
    // clang-format on

    lhs.writeTo4x4Matrix(lhsMatrix);
    rhs.value.writeTo4x4Matrix(rhsMatrix);

    return lhsMatrix[0] == Approx(rhsMatrix[0]) && lhsMatrix[4] == Approx(rhsMatrix[4]) &&
           lhsMatrix[12] == Approx(rhsMatrix[12]) && lhsMatrix[1] == Approx(rhsMatrix[1]) &&
           lhsMatrix[5] == Approx(rhsMatrix[5]) && lhsMatrix[13] == Approx(rhsMatrix[13]) &&
           lhsMatrix[3] == Approx(rhsMatrix[3]) && lhsMatrix[7] == Approx(rhsMatrix[7]) &&
           lhsMatrix[15] == Approx(rhsMatrix[15]);
}


////////////////////////////////////////////////////////////
bool operator==(const sf::Color& lhs, const Approx<sf::Color>& rhs)
{
    return sf::base::fabs(static_cast<float>(lhs.r - rhs.value.r)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.g - rhs.value.g)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.b - rhs.value.b)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.a - rhs.value.a)) < 2.f;
}
