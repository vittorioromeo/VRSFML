#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/Math/Fabs.hpp"

#include <GraphicsUtil.hpp>
#include <SystemUtil.hpp>

#include <limits>
#include <ostream>


namespace sf
{
std::ostream& operator<<(std::ostream& os, const BlendMode& blendMode)
{
    return os << "( " << static_cast<int>(blendMode.colorSrcFactor) << ", "
              << static_cast<int>(blendMode.colorDstFactor) << ", " << static_cast<int>(blendMode.colorEquation) << ", "
              << static_cast<int>(blendMode.alphaSrcFactor) << ", " << static_cast<int>(blendMode.alphaDstFactor)
              << ", " << static_cast<int>(blendMode.alphaEquation) << " )";
}

std::ostream& operator<<(std::ostream& os, const StencilComparison& comparison)
{
    switch (comparison)
    {
        case StencilComparison::Never:
            return os << "Never";
        case StencilComparison::Less:
            return os << "Less";
        case StencilComparison::LessEqual:
            return os << "LessEqual";
        case StencilComparison::Greater:
            return os << "Greater";
        case StencilComparison::GreaterEqual:
            return os << "GreaterEqual";
        case StencilComparison::Equal:
            return os << "Equal";
        case StencilComparison::NotEqual:
            return os << "NotEqual";
        case StencilComparison::Always:
            return os << "Always";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const StencilUpdateOperation& updateOperation)
{
    switch (updateOperation)
    {
        case StencilUpdateOperation::Keep:
            return os << "Keep";
        case StencilUpdateOperation::Zero:
            return os << "Zero";
        case StencilUpdateOperation::Replace:
            return os << "Replace";
        case StencilUpdateOperation::Increment:
            return os << "Increment";
        case StencilUpdateOperation::Decrement:
            return os << "Decrement";
        case StencilUpdateOperation::Invert:
            return os << "Invert";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const StencilMode& stencilMode)
{
    return os << "( " << stencilMode.stencilComparison << ", " << stencilMode.stencilUpdateOperation << ", "
              << stencilMode.stencilReference.value << ", " << stencilMode.stencilMask.value << ", "
              << stencilMode.stencilOnly << " )";
}

std::ostream& operator<<(std::ostream& os, Color color)
{
    return os << "0x" << std::hex << color.toInteger() << std::dec << " (r=" << int{color.r} << ", g=" << int{color.g}
              << ", b=" << int{color.b} << ", a=" << int{color.a} << ")";
}

std::ostream& operator<<(std::ostream& os, Approx<Color> color)
{
    return os << color.value;
}

std::ostream& operator<<(std::ostream& os, const Transform& transform)
{
    // clang-format off
    float matrix[]{{},  {},  0.f, 0.f,
                   {},  {},  0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   {},  {},  0.f, 1.f};
    // clang-format on

    transform.getMatrix(matrix);

    os << matrix[0] << ", " << matrix[4] << ", " << matrix[12] << ", ";
    os << matrix[1] << ", " << matrix[5] << ", " << matrix[13] << ", ";
    os << matrix[3] << ", " << matrix[7] << ", " << matrix[15];

    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Rect<T>& rect)
{
    const auto flags = os.flags();
    setStreamPrecision(os, std::numeric_limits<T>::max_digits10);
    os << "(position=" << rect.position << ", size=" << rect.size << ")";
    os.flags(flags);
    return os;
}

template std::ostream& operator<<(std::ostream&, const Rect<int>&);
template std::ostream& operator<<(std::ostream&, const Rect<float>&);

} // namespace sf

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

    lhs.getMatrix(lhsMatrix);
    rhs.value.getMatrix(rhsMatrix);

    return lhsMatrix[0] == Approx(rhsMatrix[0]) && lhsMatrix[4] == Approx(rhsMatrix[4]) &&
           lhsMatrix[12] == Approx(rhsMatrix[12]) && lhsMatrix[1] == Approx(rhsMatrix[1]) &&
           lhsMatrix[5] == Approx(rhsMatrix[5]) && lhsMatrix[13] == Approx(rhsMatrix[13]) &&
           lhsMatrix[3] == Approx(rhsMatrix[3]) && lhsMatrix[7] == Approx(rhsMatrix[7]) &&
           lhsMatrix[15] == Approx(rhsMatrix[15]);
}

bool operator==(const sf::Color& lhs, const Approx<sf::Color>& rhs)
{
    return sf::base::fabs(static_cast<float>(lhs.r - rhs.value.r)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.g - rhs.value.g)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.b - rhs.value.b)) < 2.f &&
           sf::base::fabs(static_cast<float>(lhs.a - rhs.value.a)) < 2.f;
}
