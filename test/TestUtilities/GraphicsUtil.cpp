#include "GraphicsViewUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Rect2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"


namespace
{
////////////////////////////////////////////////////////////
sf::base::String gfxFloatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<sf::base::SizeT>(end - buf);
    return sf::base::String{buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String gfxIntToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<sf::base::SizeT>(end - buf);
    return sf::base::String{buf, len};
}


////////////////////////////////////////////////////////////
sf::base::String hexToString(const sf::base::U32 value)
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
    return sf::base::String{buf, static_cast<sf::base::SizeT>(pos)};
}


////////////////////////////////////////////////////////////
sf::base::String stencilComparisonToString(const sf::StencilComparison comparison)
{
    switch (comparison)
    {
        case sf::StencilComparison::Never:
            return sf::base::String{"Never"};
        case sf::StencilComparison::Less:
            return sf::base::String{"Less"};
        case sf::StencilComparison::LessEqual:
            return sf::base::String{"LessEqual"};
        case sf::StencilComparison::Greater:
            return sf::base::String{"Greater"};
        case sf::StencilComparison::GreaterEqual:
            return sf::base::String{"GreaterEqual"};
        case sf::StencilComparison::Equal:
            return sf::base::String{"Equal"};
        case sf::StencilComparison::NotEqual:
            return sf::base::String{"NotEqual"};
        case sf::StencilComparison::Always:
            return sf::base::String{"Always"};
    }
    return sf::base::String{};
}


////////////////////////////////////////////////////////////
sf::base::String stencilUpdateOperationToString(const sf::StencilUpdateOperation updateOperation)
{
    switch (updateOperation)
    {
        case sf::StencilUpdateOperation::Keep:
            return sf::base::String{"Keep"};
        case sf::StencilUpdateOperation::Zero:
            return sf::base::String{"Zero"};
        case sf::StencilUpdateOperation::Replace:
            return sf::base::String{"Replace"};
        case sf::StencilUpdateOperation::Increment:
            return sf::base::String{"Increment"};
        case sf::StencilUpdateOperation::Decrement:
            return sf::base::String{"Decrement"};
        case sf::StencilUpdateOperation::Invert:
            return sf::base::String{"Invert"};
    }
    return sf::base::String{};
}


////////////////////////////////////////////////////////////
sf::base::String vec2ToString(const sf::Vec2<float> v)
{
    return sf::base::String{"("} + gfxFloatToString(v.x) + sf::base::String{", "} + gfxFloatToString(v.y) +
           sf::base::String{")"};
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String rectToString(const sf::Rect2<T>& rect)
{
    if constexpr (sf::base::isFloatingPoint<T>)
        return sf::base::String{"(position=("} + gfxFloatToString(rect.position.x) + sf::base::String{", "} +
               gfxFloatToString(rect.position.y) + sf::base::String{"), size=("} + gfxFloatToString(rect.size.x) +
               sf::base::String{", "} + gfxFloatToString(rect.size.y) + sf::base::String{"))"};
    else
        return sf::base::String{"(position=("} + gfxIntToString(rect.position.x) + sf::base::String{", "} +
               gfxIntToString(rect.position.y) + sf::base::String{"), size=("} + gfxIntToString(rect.size.x) +
               sf::base::String{", "} + gfxIntToString(rect.size.y) + sf::base::String{"))"};
}


////////////////////////////////////////////////////////////
sf::base::String blendModeToString(const sf::BlendMode& blendMode)
{
    return sf::base::String{"( "} + gfxIntToString(static_cast<int>(blendMode.colorSrcFactor)) +
           sf::base::String{", "} + gfxIntToString(static_cast<int>(blendMode.colorDstFactor)) +
           sf::base::String{", "} + gfxIntToString(static_cast<int>(blendMode.colorEquation)) + sf::base::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaSrcFactor)) + sf::base::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaDstFactor)) + sf::base::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaEquation)) + sf::base::String{" )"};
}


////////////////////////////////////////////////////////////
sf::base::String stencilModeToString(const sf::StencilMode& stencilMode)
{
    return sf::base::String{"( "} + stencilComparisonToString(stencilMode.stencilComparison) + sf::base::String{", "} +
           stencilUpdateOperationToString(stencilMode.stencilUpdateOperation) + sf::base::String{", "} +
           sf::base::String{stencilMode.stencilOnly ? "true" : "false"} + sf::base::String{", "} +
           gfxIntToString(static_cast<unsigned int>(stencilMode.stencilReference.value)) + sf::base::String{", "} +
           gfxIntToString(static_cast<unsigned int>(stencilMode.stencilMask.value)) + sf::base::String{" )"};
}


////////////////////////////////////////////////////////////
sf::base::String colorToString(const sf::Color color)
{
    return hexToString(color.toInteger()) + sf::base::String{" (r="} + gfxIntToString(int{color.r}) +
           sf::base::String{", g="} + gfxIntToString(int{color.g}) + sf::base::String{", b="} +
           gfxIntToString(int{color.b}) + sf::base::String{", a="} + gfxIntToString(int{color.a}) +
           sf::base::String{")"};
}


////////////////////////////////////////////////////////////
sf::base::String transformToString(const sf::Transform& transform)
{
    // clang-format off
    float matrix[]{{},  {},  0.f, 0.f,
                   {},  {},  0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   {},  {},  0.f, 1.f};
    // clang-format on

    transform.writeTo4x4Matrix(matrix);

    return gfxFloatToString(matrix[0]) + sf::base::String{", "} + gfxFloatToString(matrix[4]) + sf::base::String{", "} +
           gfxFloatToString(matrix[12]) + sf::base::String{", "} + gfxFloatToString(matrix[1]) +
           sf::base::String{", "} + gfxFloatToString(matrix[5]) + sf::base::String{", "} +
           gfxFloatToString(matrix[13]) + sf::base::String{", "} + gfxFloatToString(matrix[3]) +
           sf::base::String{", "} + gfxFloatToString(matrix[7]) + sf::base::String{", "} + gfxFloatToString(matrix[15]);
}


////////////////////////////////////////////////////////////
sf::base::String viewToString(const sf::View& view)
{
    return sf::base::String{"( center="} + vec2ToString(view.center) + sf::base::String{", size="} +
           vec2ToString(view.size) + sf::base::String{", rotation="} + gfxFloatToString(view.rotation.asDegrees()) +
           sf::base::String{" deg"} + sf::base::String{", viewport="} + rectToString(view.viewport) +
           sf::base::String{", scissor="} + rectToString(view.scissor) + sf::base::String{" )"};
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::BlendMode& blendMode) noexcept
{
    const sf::base::String s = blendModeToString(blendMode);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::StencilComparison comparison) noexcept
{
    const sf::base::String s = stencilComparisonToString(comparison);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::StencilUpdateOperation updateOperation) noexcept
{
    const sf::base::String s = stencilUpdateOperationToString(updateOperation);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::StencilMode& stencilMode) noexcept
{
    const sf::base::String s = stencilModeToString(stencilMode);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Color& color) noexcept
{
    const sf::base::String s = colorToString(color);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Transform& transform) noexcept
{
    const sf::base::String s = transformToString(transform);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::View& view) noexcept
{
    const sf::base::String s = viewToString(view);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::View::ScissorRect& scissorRect) noexcept
{
    const sf::base::String s = rectToString(static_cast<const sf::Rect2<float>&>(scissorRect));
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace sf


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
