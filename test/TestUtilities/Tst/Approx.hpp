#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/StringifyValue.hpp"


namespace tst
{
////////////////////////////////////////////////////////////
/// \brief Catch/doctest-style approximate float comparison.
///
/// Comparison body in `Approx.cpp` so we never re-emit `fabs` /
/// epsilon-scaling per translation unit.
////////////////////////////////////////////////////////////
class Approx
{
public:
    explicit Approx(double value) noexcept;

    Approx& epsilon(double newEpsilon) noexcept;
    Approx& scale(double newScale) noexcept;
    Approx& margin(double newMargin) noexcept;

    [[nodiscard]] double value() const noexcept;
    [[nodiscard]] double epsilonValue() const noexcept;
    [[nodiscard]] double marginValue() const noexcept;
    [[nodiscard]] double scaleValue() const noexcept;

    friend bool operator==(double lhs, const Approx& rhs) noexcept;
    friend bool operator==(const Approx& lhs, double rhs) noexcept;
    friend bool operator!=(double lhs, const Approx& rhs) noexcept;
    friend bool operator!=(const Approx& lhs, double rhs) noexcept;

    friend bool operator==(float lhs, const Approx& rhs) noexcept;
    friend bool operator==(const Approx& lhs, float rhs) noexcept;
    friend bool operator!=(float lhs, const Approx& rhs) noexcept;
    friend bool operator!=(const Approx& lhs, float rhs) noexcept;

private:
    double m_epsilon;
    double m_margin;
    double m_scale;
    double m_value;
};


namespace Matchers
{
////////////////////////////////////////////////////////////
/// \brief Catch::Matchers::WithinRel shim. Used in a couple of tests
/// that came over from Catch2.
////////////////////////////////////////////////////////////
struct WithinRel
{
    double target;
    double epsilon;
};

struct WithinAbs
{
    double target;
    double epsilon;
};

[[nodiscard]] bool operator==(float value, const WithinRel& m) noexcept;
[[nodiscard]] bool operator==(double value, const WithinRel& m) noexcept;
[[nodiscard]] bool operator==(float value, const WithinAbs& m) noexcept;
[[nodiscard]] bool operator==(double value, const WithinAbs& m) noexcept;
} // namespace Matchers

} // namespace tst
