#include "GraphicsViewUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/BlendMode.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/StencilMode.hpp"
#include "Zancle/Graphics/Transform.hpp"
#include "Zancle/Graphics/View.hpp"

#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToChars.hpp"
#include "ZancleBase/Trait/IsFloatingPoint.hpp"


namespace
{
////////////////////////////////////////////////////////////
zb::String gfxFloatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = zb::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<zb::SizeT>(end - buf);
    return zb::String{buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
zb::String gfxIntToString(const T value)
{
    char       buf[32];
    char*      end = zb::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<zb::SizeT>(end - buf);
    return zb::String{buf, len};
}


////////////////////////////////////////////////////////////
zb::String hexToString(const zb::U32 value)
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
    return zb::String{buf, static_cast<zb::SizeT>(pos)};
}


////////////////////////////////////////////////////////////
zb::String stencilComparisonToString(const za::StencilComparison comparison)
{
    switch (comparison)
    {
        case za::StencilComparison::Never:
            return zb::String{"Never"};
        case za::StencilComparison::Less:
            return zb::String{"Less"};
        case za::StencilComparison::LessEqual:
            return zb::String{"LessEqual"};
        case za::StencilComparison::Greater:
            return zb::String{"Greater"};
        case za::StencilComparison::GreaterEqual:
            return zb::String{"GreaterEqual"};
        case za::StencilComparison::Equal:
            return zb::String{"Equal"};
        case za::StencilComparison::NotEqual:
            return zb::String{"NotEqual"};
        case za::StencilComparison::Always:
            return zb::String{"Always"};
    }
    return zb::String{};
}


////////////////////////////////////////////////////////////
zb::String stencilUpdateOperationToString(const za::StencilUpdateOperation updateOperation)
{
    switch (updateOperation)
    {
        case za::StencilUpdateOperation::Keep:
            return zb::String{"Keep"};
        case za::StencilUpdateOperation::Zero:
            return zb::String{"Zero"};
        case za::StencilUpdateOperation::Replace:
            return zb::String{"Replace"};
        case za::StencilUpdateOperation::Increment:
            return zb::String{"Increment"};
        case za::StencilUpdateOperation::Decrement:
            return zb::String{"Decrement"};
        case za::StencilUpdateOperation::Invert:
            return zb::String{"Invert"};
    }
    return zb::String{};
}


////////////////////////////////////////////////////////////
zb::String vec2ToString(const za::Vec2<float> v)
{
    return zb::String{"("} + gfxFloatToString(v.x) + zb::String{", "} + gfxFloatToString(v.y) + zb::String{")"};
}


////////////////////////////////////////////////////////////
template <typename T>
zb::String rectToString(const za::Rect2<T>& rect)
{
    if constexpr (zb::isFloatingPoint<T>)
        return zb::String{"(position=("} + gfxFloatToString(rect.position.x) + zb::String{", "} +
               gfxFloatToString(rect.position.y) + zb::String{"), size=("} + gfxFloatToString(rect.size.x) +
               zb::String{", "} + gfxFloatToString(rect.size.y) + zb::String{"))"};
    else
        return zb::String{"(position=("} + gfxIntToString(rect.position.x) + zb::String{", "} +
               gfxIntToString(rect.position.y) + zb::String{"), size=("} + gfxIntToString(rect.size.x) +
               zb::String{", "} + gfxIntToString(rect.size.y) + zb::String{"))"};
}


////////////////////////////////////////////////////////////
zb::String blendModeToString(const za::BlendMode& blendMode)
{
    return zb::String{"( "} + gfxIntToString(static_cast<int>(blendMode.colorSrcFactor)) + zb::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.colorDstFactor)) + zb::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.colorEquation)) + zb::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaSrcFactor)) + zb::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaDstFactor)) + zb::String{", "} +
           gfxIntToString(static_cast<int>(blendMode.alphaEquation)) + zb::String{" )"};
}


////////////////////////////////////////////////////////////
zb::String stencilModeToString(const za::StencilMode& stencilMode)
{
    return zb::String{"( "} + stencilComparisonToString(stencilMode.stencilComparison) + zb::String{", "} +
           stencilUpdateOperationToString(stencilMode.stencilUpdateOperation) + zb::String{", "} +
           zb::String{stencilMode.stencilOnly ? "true" : "false"} + zb::String{", "} +
           gfxIntToString(static_cast<unsigned int>(stencilMode.stencilReference.value)) + zb::String{", "} +
           gfxIntToString(static_cast<unsigned int>(stencilMode.stencilMask.value)) + zb::String{" )"};
}


////////////////////////////////////////////////////////////
zb::String colorToString(const za::Color color)
{
    return hexToString(color.toInteger()) + zb::String{" (r="} + gfxIntToString(int{color.r}) + zb::String{", g="} +
           gfxIntToString(int{color.g}) + zb::String{", b="} + gfxIntToString(int{color.b}) + zb::String{", a="} +
           gfxIntToString(int{color.a}) + zb::String{")"};
}


////////////////////////////////////////////////////////////
zb::String transformToString(const za::Transform& transform)
{
    // clang-format off
    float matrix[]{{},  {},  0.f, 0.f,
                   {},  {},  0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   {},  {},  0.f, 1.f};
    // clang-format on

    transform.writeTo4x4Matrix(matrix);

    return gfxFloatToString(matrix[0]) + zb::String{", "} + gfxFloatToString(matrix[4]) + zb::String{", "} +
           gfxFloatToString(matrix[12]) + zb::String{", "} + gfxFloatToString(matrix[1]) + zb::String{", "} +
           gfxFloatToString(matrix[5]) + zb::String{", "} + gfxFloatToString(matrix[13]) + zb::String{", "} +
           gfxFloatToString(matrix[3]) + zb::String{", "} + gfxFloatToString(matrix[7]) + zb::String{", "} +
           gfxFloatToString(matrix[15]);
}


////////////////////////////////////////////////////////////
zb::String viewToString(const za::View& view)
{
    return zb::String{"( center="} + vec2ToString(view.center) + zb::String{", size="} + vec2ToString(view.size) +
           zb::String{", rotation="} + gfxFloatToString(view.rotation.asDegrees()) + zb::String{" deg"} +
           zb::String{", viewport="} + rectToString(view.viewport) + zb::String{", scissor="} +
           rectToString(view.scissor) + zb::String{" )"};
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::BlendMode& blendMode) noexcept
{
    const zb::String s = blendModeToString(blendMode);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::StencilComparison comparison) noexcept
{
    const zb::String s = stencilComparisonToString(comparison);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::StencilUpdateOperation updateOperation) noexcept
{
    const zb::String s = stencilUpdateOperationToString(updateOperation);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::StencilMode& stencilMode) noexcept
{
    const zb::String s = stencilModeToString(stencilMode);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Color& color) noexcept
{
    const zb::String s = colorToString(color);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Transform& transform) noexcept
{
    const zb::String s = transformToString(transform);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::View& view) noexcept
{
    const zb::String s = viewToString(view);
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::View::ScissorRect& scissorRect) noexcept
{
    const zb::String s = rectToString(static_cast<const za::Rect2<float>&>(scissorRect));
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace za


////////////////////////////////////////////////////////////
bool operator==(const za::Transform& lhs, const Approx<za::Transform>& rhs)
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
bool operator==(const za::Color& lhs, const Approx<za::Color>& rhs)
{
    return zb::fabs(static_cast<float>(lhs.r - rhs.value.r)) < 2.f &&
           zb::fabs(static_cast<float>(lhs.g - rhs.value.g)) < 2.f &&
           zb::fabs(static_cast<float>(lhs.b - rhs.value.b)) < 2.f &&
           zb::fabs(static_cast<float>(lhs.a - rhs.value.a)) < 2.f;
}
