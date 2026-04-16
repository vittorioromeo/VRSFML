#include "SystemUtil.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/ChevronShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/CogShapeData.hpp"
#include "SFML/Graphics/CrossShapeData.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/HeartShapeData.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/TrapezoidShapeData.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SinCosLookup.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
// Helper: assert the four corners of `a` approximately equal the four corners of `b`
// (comparing the min/max corners suffices since Rect2f stores position + size).
////////////////////////////////////////////////////////////
[[nodiscard]] bool rectsApproxEqual(const sf::Rect2f& a, const sf::Rect2f& b) noexcept
{
    return a.position == Approx(b.position) && a.size == Approx(b.size);
}


////////////////////////////////////////////////////////////
// Numerical polygon centroid via shoelace -- ground truth for shape-data `getCentroid`
// formulas. The polygon is specified by `count` vertices obtained from `pointFn(i)`.
////////////////////////////////////////////////////////////
template <typename PointFn>
[[nodiscard]] sf::Vec2f referencePolygonCentroid(const unsigned int count, PointFn&& pointFn) noexcept
{
    float     area2 = 0.f;
    sf::Vec2f acc{};

    sf::Vec2f prev = pointFn(0u);
    for (unsigned int i = 0u; i < count; ++i)
    {
        const sf::Vec2f curr  = pointFn((i + 1u) % count);
        const float     cross = prev.x * curr.y - curr.x * prev.y;
        area2 += cross;
        acc.x += (prev.x + curr.x) * cross;
        acc.y += (prev.y + curr.y) * cross;
        prev = curr;
    }

    if (area2 == 0.f)
        return {};

    return {acc.x / (3.f * area2), acc.y / (3.f * area2)};
}
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::ShapeData type traits")
{
#define SFML_PRIV_CHECK_TRAITS(T)                                 \
    STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::T));         \
    STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::T));            \
    STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::T)); \
    STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::T))

    SFML_PRIV_CHECK_TRAITS(ArrowShapeData);
    SFML_PRIV_CHECK_TRAITS(ChevronShapeData);
    SFML_PRIV_CHECK_TRAITS(CircleShapeData);
    SFML_PRIV_CHECK_TRAITS(CogShapeData);
    SFML_PRIV_CHECK_TRAITS(CrossShapeData);
    SFML_PRIV_CHECK_TRAITS(CurvedArrowShapeData);
    SFML_PRIV_CHECK_TRAITS(EllipseShapeData);
    SFML_PRIV_CHECK_TRAITS(HeartShapeData);
    SFML_PRIV_CHECK_TRAITS(PieSliceShapeData);
    SFML_PRIV_CHECK_TRAITS(RectangleShapeData);
    SFML_PRIV_CHECK_TRAITS(RingPieSliceShapeData);
    SFML_PRIV_CHECK_TRAITS(RingShapeData);
    SFML_PRIV_CHECK_TRAITS(RoundedRectangleShapeData);
    SFML_PRIV_CHECK_TRAITS(StarShapeData);
    SFML_PRIV_CHECK_TRAITS(TrapezoidShapeData);

#undef SFML_PRIV_CHECK_TRAITS
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::CircleShapeData")
{
    SECTION("Default local bounds")
    {
        const sf::CircleShapeData sd{};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {0.f, 0.f}}));
    }

    SECTION("Local bounds match 2*radius square")
    {
        const sf::CircleShapeData sd{.radius = 10.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {20.f, 20.f}}));
    }

    SECTION("Global bounds: identity transform equals local bounds")
    {
        const sf::CircleShapeData sd{.radius = 10.f};
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), sd.getLocalBounds()));
    }

    SECTION("Global bounds: translated by position only")
    {
        const sf::CircleShapeData sd{.position = {100.f, 50.f}, .radius = 10.f};
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {{100.f, 50.f}, {20.f, 20.f}}));
    }

    SECTION("Global bounds: rotation-invariant under uniform scale")
    {
        const sf::CircleShapeData unrotated{.radius = 10.f};
        const sf::CircleShapeData rotated{.rotation = sf::degrees(45.f), .radius = 10.f};
        // Without origin, rotation is about (0, 0) so the bbox position moves, but its size stays.
        CHECK(rotated.getGlobalBounds().size == Approx(unrotated.getGlobalBounds().size));
    }

    SECTION("Global bounds: rotation about center keeps bounds identical")
    {
        const sf::CircleShapeData sd{.origin = {10.f, 10.f}, .rotation = sf::degrees(33.f), .radius = 10.f};
        // Rotating a disk about its own center is a no-op.
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {{-10.f, -10.f}, {20.f, 20.f}}));
    }

    SECTION("Anchor points")
    {
        const sf::CircleShapeData sd{.radius = 10.f};
        CHECK(sd.getLocalCenter() == Approx(sf::Vec2f{10.f, 10.f}));
        CHECK(sd.getLocalTopLeft() == Approx(sf::Vec2f{0.f, 0.f}));
        CHECK(sd.getLocalBottomRight() == Approx(sf::Vec2f{20.f, 20.f}));
    }

    SECTION("Centroid: coincides with disk center")
    {
        const sf::CircleShapeData sd{.radius = 10.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{10.f, 10.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::EllipseShapeData")
{
    SECTION("Local bounds reflect both radii")
    {
        const sf::EllipseShapeData sd{.horizontalRadius = 20.f, .verticalRadius = 10.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 20.f}}));
    }

    SECTION("Global bounds: 90 deg rotation about center swaps dimensions")
    {
        const sf::EllipseShapeData sd{.origin           = {20.f, 10.f},
                                      .rotation         = sf::degrees(90.f),
                                      .horizontalRadius = 20.f,
                                      .verticalRadius   = 10.f};
        // After a 90 deg rotation, the horizontal and vertical extents swap.
        const auto gb = sd.getGlobalBounds();
        CHECK(gb.size == Approx(sf::Vec2f{20.f, 40.f}));
    }

    SECTION("Global bounds: uniform scale scales bounds uniformly")
    {
        const sf::EllipseShapeData sd{.scale = {2.f, 2.f}, .horizontalRadius = 10.f, .verticalRadius = 5.f};
        CHECK(sd.getGlobalBounds().size == Approx(sf::Vec2f{40.f, 20.f}));
    }

    SECTION("Centroid: coincides with ellipse center")
    {
        const sf::EllipseShapeData sd{.horizontalRadius = 20.f, .verticalRadius = 10.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 10.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::RingShapeData")
{
    SECTION("Local bounds ignore inner radius")
    {
        const sf::RingShapeData sd{.outerRadius = 30.f, .innerRadius = 10.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {60.f, 60.f}}));
    }

    SECTION("Global bounds: tight under rotation")
    {
        const sf::RingShapeData sd{.origin = {30.f, 30.f}, .rotation = sf::degrees(45.f), .outerRadius = 30.f, .innerRadius = 15.f};
        // Ring is rotation-invariant around its own center: bounds should stay a 60x60 square.
        CHECK(sd.getGlobalBounds().size == Approx(sf::Vec2f{60.f, 60.f}));
    }

    SECTION("Invalid inner radius >= outer radius yields an empty bounding box")
    {
        const sf::RingShapeData sd{.outerRadius = 10.f, .innerRadius = 20.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Non-positive outer radius yields an empty bounding box")
    {
        const sf::RingShapeData sd{.outerRadius = 0.f, .innerRadius = 0.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Centroid: coincides with ring center")
    {
        const sf::RingShapeData sd{.outerRadius = 30.f, .innerRadius = 10.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{30.f, 30.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::RectangleShapeData")
{
    SECTION("Local bounds")
    {
        const sf::RectangleShapeData sd{.size = {40.f, 20.f}};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 20.f}}));
    }

    SECTION("Global bounds with translation")
    {
        const sf::RectangleShapeData sd{.position = {5.f, 7.f}, .size = {40.f, 20.f}};
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {{5.f, 7.f}, {40.f, 20.f}}));
    }

    SECTION("Global bounds: 90 deg rotation about center swaps dimensions exactly")
    {
        const sf::RectangleShapeData sd{.origin = {20.f, 10.f}, .rotation = sf::degrees(90.f), .size = {40.f, 20.f}};
        CHECK(sd.getGlobalBounds().size == Approx(sf::Vec2f{20.f, 40.f}));
    }

    SECTION("Global bounds: 45 deg rotated rectangle is tight")
    {
        const sf::RectangleShapeData sd{.origin = {10.f, 10.f}, .rotation = sf::degrees(45.f), .size = {20.f, 20.f}};
        // A 20x20 square rotated 45 deg has an AABB of 20*sqrt(2) ~ 28.2843 on each side.
        const float expected = 20.f * 1.41421356f;
        CHECK(sd.getGlobalBounds().size.x == Approx(expected));
        CHECK(sd.getGlobalBounds().size.y == Approx(expected));
    }

    SECTION("Scale doubles both extents")
    {
        const sf::RectangleShapeData sd{.scale = {2.f, 3.f}, .size = {10.f, 10.f}};
        CHECK(sd.getGlobalBounds().size == Approx(sf::Vec2f{20.f, 30.f}));
    }

    SECTION("Centroid: coincides with rectangle center")
    {
        const sf::RectangleShapeData sd{.size = {40.f, 20.f}};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 10.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::RoundedRectangleShapeData")
{
    SECTION("Local bounds")
    {
        const sf::RoundedRectangleShapeData sd{.size = {40.f, 20.f}, .cornerRadius = 5.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 20.f}}));
    }

    SECTION("Global bounds with identity transform")
    {
        const sf::RoundedRectangleShapeData sd{.size = {40.f, 20.f}, .cornerRadius = 5.f, .cornerPointCount = 32u};
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), sd.getLocalBounds()));
    }

    SECTION("Centroid: coincides with rectangle center")
    {
        const sf::RoundedRectangleShapeData sd{.size = {40.f, 20.f}, .cornerRadius = 5.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 10.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::StarShapeData")
{
    SECTION("Local bounds are 2 * outerRadius square")
    {
        const sf::StarShapeData sd{.outerRadius = 30.f, .innerRadius = 15.f, .pointCount = 5u};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {60.f, 60.f}}));
    }

    SECTION("Global bounds never exceed the outer disk bbox")
    {
        const sf::StarShapeData sd{.origin      = {30.f, 30.f},
                                   .rotation    = sf::degrees(17.f),
                                   .outerRadius = 30.f,
                                   .innerRadius = 15.f,
                                   .pointCount  = 5u};
        const auto              gb = sd.getGlobalBounds();
        // Tight global bounds cannot exceed the disk enclosing the star tips.
        CHECK(gb.size.x <= 60.f + 1e-3f);
        CHECK(gb.size.y <= 60.f + 1e-3f);
    }

    SECTION("Centroid: coincides with star center")
    {
        const sf::StarShapeData sd{.outerRadius = 30.f, .innerRadius = 15.f, .pointCount = 5u};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{30.f, 30.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::CogShapeData")
{
    SECTION("Local bounds are 2 * outerRadius square")
    {
        const sf::CogShapeData sd{.outerRadius = 40.f, .innerRadius = 20.f, .toothCount = 8u};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {80.f, 80.f}}));
    }

    SECTION("Global bounds never exceed the outer disk bbox under rotation")
    {
        const sf::CogShapeData sd{.origin          = {40.f, 40.f},
                                  .rotation        = sf::degrees(23.f),
                                  .outerRadius     = 40.f,
                                  .innerRadius     = 20.f,
                                  .toothCount      = 12u,
                                  .toothWidthRatio = 0.5f};
        const auto             gb = sd.getGlobalBounds();
        CHECK(gb.size.x <= 80.f + 1e-3f);
        CHECK(gb.size.y <= 80.f + 1e-3f);
    }

    SECTION("Centroid: coincides with cog center")
    {
        const sf::CogShapeData sd{.outerRadius = 40.f, .innerRadius = 20.f, .toothCount = 8u};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{40.f, 40.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::PieSliceShapeData")
{
    SECTION("Degenerate sweep yields a point at the hub")
    {
        const sf::PieSliceShapeData sd{.radius = 10.f, .startAngle = sf::degrees(0.f), .sweepAngle = sf::degrees(0.f)};
        // Nothing is drawn, so bounds are empty.
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Full circle sweep produces full-disk local bounds")
    {
        const sf::PieSliceShapeData sd{.radius     = 10.f,
                                       .startAngle = sf::degrees(0.f),
                                       .sweepAngle = sf::radians(sf::base::tau)};
        // With sweep == tau, every cardinal is in range -- bbox fills the enclosing square.
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {20.f, 20.f}}));
    }

    SECTION("Narrow sweep does not reach the far side of the disk")
    {
        const sf::PieSliceShapeData sd{.radius = 10.f, .startAngle = sf::degrees(0.f), .sweepAngle = sf::degrees(10.f)};
        // A thin wedge starting at angle 0 cannot span both axes.
        const auto lb = sd.getLocalBounds();
        CHECK(lb.size.x <= 20.f);
        CHECK(lb.size.y <= 20.f);
        CHECK((lb.size.x < 20.f || lb.size.y < 20.f));
    }

    SECTION("Centroid: full-circle sweep collapses to the hub")
    {
        // With d = (4*R*sin(pi)) / (3*tau) = 0, the centroid sits exactly on the hub.
        const sf::PieSliceShapeData sd{.radius = 10.f, .sweepAngle = sf::radians(sf::base::tau)};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{10.f, 10.f}));
    }

    SECTION("Centroid: degenerate sweep falls back to the hub")
    {
        const sf::PieSliceShapeData sd{.radius = 10.f, .sweepAngle = sf::degrees(0.f)};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{10.f, 10.f}));
    }

    SECTION("Centroid: agrees with polygon shoelace reference on a 90 deg wedge")
    {
        const sf::PieSliceShapeData sd{.radius     = 10.f,
                                       .startAngle = sf::degrees(0.f),
                                       .sweepAngle = sf::degrees(90.f),
                                       .pointCount = 256u};
        // Numerical reference on the tessellated polygon (dense enough to closely
        // approximate the true sector).
        const auto reference = referencePolygonCentroid(sd.pointCount,
                                                        [&](const unsigned int i) noexcept
        {
            if (i == 0u)
                return sf::Vec2f{sd.radius, sd.radius};

            const float step  = sd.sweepAngle.asRadians() / static_cast<float>(sd.pointCount - 2u);
            const float angle = sd.startAngle.asRadians() + static_cast<float>(i - 1u) * step;
            const auto  sc    = sf::base::sinCosLookup(sf::base::positiveRemainder(angle, sf::base::tau));
            return sf::Vec2f{sd.radius - sd.radius * sc.sin, sd.radius + sd.radius * sc.cos};
        });

        // Tolerance reflects the finite tessellation of the reference polygon.
        CHECK(sf::base::fabs(sd.getCentroid().x - reference.x) < 0.05f);
        CHECK(sf::base::fabs(sd.getCentroid().y - reference.y) < 0.05f);
    }

    SECTION("Negative sweep is equivalent to positive sweep with shifted start")
    {
        // `startAngle + sweep` ending at the same angle in both cases should produce
        // identical bounds and centroid (the shape is the same, traversed in reverse).
        const sf::PieSliceShapeData forward{.radius     = 10.f,
                                            .startAngle = sf::degrees(30.f),
                                            .sweepAngle = sf::degrees(120.f),
                                            .pointCount = 64u};

        const sf::PieSliceShapeData backward{.radius     = 10.f,
                                             .startAngle = sf::degrees(150.f),
                                             .sweepAngle = sf::degrees(-120.f),
                                             .pointCount = 64u};

        CHECK(backward.hasVisibleGeometry());
        CHECK(rectsApproxEqual(forward.getLocalBounds(), backward.getLocalBounds()));
        CHECK(forward.getCentroid() == Approx(backward.getCentroid()));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::RingPieSliceShapeData")
{
    SECTION("Full sweep produces full outer-ring bbox")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 30.f,
                                           .innerRadius = 15.f,
                                           .startAngle  = sf::degrees(0.f),
                                           .sweepAngle  = sf::radians(sf::base::tau)};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {60.f, 60.f}}));
    }

    SECTION("Degenerate sweep yields an empty bounding box (nothing drawn)")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 30.f, .innerRadius = 15.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Invalid inner radius >= outer radius yields an empty bounding box")
    {
        // `add(...)` early-returns for this case; bounds should agree.
        const sf::RingPieSliceShapeData sd{.outerRadius = 10.f,
                                           .innerRadius = 20.f,
                                           .startAngle  = sf::degrees(0.f),
                                           .sweepAngle  = sf::degrees(90.f)};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Non-positive outer radius yields an empty bounding box")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 0.f, .innerRadius = 0.f, .sweepAngle = sf::degrees(90.f)};
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Centroid: full-circle sweep collapses to the ring center")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 30.f, .innerRadius = 15.f, .sweepAngle = sf::radians(sf::base::tau)};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{30.f, 30.f}));
    }

    SECTION("Centroid: degenerate sweep falls back to the ring center")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 30.f, .innerRadius = 15.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{30.f, 30.f}));
    }

    SECTION("Centroid: agrees with polygon shoelace reference on a 90 deg ring wedge")
    {
        const sf::RingPieSliceShapeData sd{.outerRadius = 30.f,
                                           .innerRadius = 15.f,
                                           .startAngle  = sf::degrees(0.f),
                                           .sweepAngle  = sf::degrees(90.f),
                                           .pointCount  = 256u};

        // Simple closed polygon for the sector: outer arc forward, then inner arc reversed.
        const auto reference = referencePolygonCentroid(2u * sd.pointCount,
                                                        [&](const unsigned int i) noexcept
        {
            const float        step      = sd.sweepAngle.asRadians() / static_cast<float>(sd.pointCount - 1u);
            const bool         outerSide = (i < sd.pointCount);
            const unsigned int local     = outerSide ? i : (sd.pointCount - 1u - (i - sd.pointCount));
            const float        radius    = outerSide ? sd.outerRadius : sd.innerRadius;
            const float        angle     = sd.startAngle.asRadians() + static_cast<float>(local) * step;
            const auto         sc        = sf::base::sinCosLookup(sf::base::positiveRemainder(angle, sf::base::tau));
            return sf::Vec2f{sd.outerRadius + radius * sc.cos, sd.outerRadius + radius * sc.sin};
        });

        CHECK(sf::base::fabs(sd.getCentroid().x - reference.x) < 0.1f);
        CHECK(sf::base::fabs(sd.getCentroid().y - reference.y) < 0.1f);
    }

    SECTION("Negative sweep is equivalent to positive sweep with shifted start")
    {
        const sf::RingPieSliceShapeData forward{.outerRadius = 30.f,
                                                .innerRadius = 15.f,
                                                .startAngle  = sf::degrees(30.f),
                                                .sweepAngle  = sf::degrees(120.f),
                                                .pointCount  = 64u};

        const sf::RingPieSliceShapeData backward{.outerRadius = 30.f,
                                                 .innerRadius = 15.f,
                                                 .startAngle  = sf::degrees(150.f),
                                                 .sweepAngle  = sf::degrees(-120.f),
                                                 .pointCount  = 64u};

        CHECK(backward.hasVisibleGeometry());
        CHECK(rectsApproxEqual(forward.getLocalBounds(), backward.getLocalBounds()));
        CHECK(forward.getCentroid() == Approx(backward.getCentroid()));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::CurvedArrowShapeData")
{
    SECTION("Zero-sweep arrow yields an empty bounding box (nothing drawn)")
    {
        const sf::CurvedArrowShapeData
            sd{.outerRadius = 20.f, .innerRadius = 10.f, .sweepAngle = sf::degrees(0.f), .headLength = 5.f, .headWidth = 8.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Invalid inner >= outer radius yields an empty bounding box")
    {
        const sf::CurvedArrowShapeData sd{.outerRadius = 10.f,
                                          .innerRadius = 20.f,
                                          .sweepAngle  = sf::degrees(90.f),
                                          .headLength  = 5.f,
                                          .headWidth   = 8.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Local bounds respect sweep")
    {
        const sf::CurvedArrowShapeData narrow{.outerRadius = 30.f,
                                              .innerRadius = 15.f,
                                              .startAngle  = sf::degrees(0.f),
                                              .sweepAngle  = sf::degrees(30.f),
                                              .headLength  = 5.f,
                                              .headWidth   = 8.f};
        const sf::CurvedArrowShapeData wide{.outerRadius = 30.f,
                                            .innerRadius = 15.f,
                                            .startAngle  = sf::degrees(0.f),
                                            .sweepAngle  = sf::degrees(180.f),
                                            .headLength  = 5.f,
                                            .headWidth   = 8.f};
        // Wider arc should have at least as much extent in some axis as a narrower one.
        CHECK(wide.getLocalBounds().size.x + wide.getLocalBounds().size.y >=
              narrow.getLocalBounds().size.x + narrow.getLocalBounds().size.y);
    }

    SECTION("Centroid: degenerate shape falls back to the ring center")
    {
        const sf::CurvedArrowShapeData
            sd{.outerRadius = 20.f, .innerRadius = 10.f, .sweepAngle = sf::degrees(0.f), .headLength = 5.f, .headWidth = 8.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 20.f}));
    }

    SECTION("Centroid: lies inside the shape's bounding box")
    {
        const sf::CurvedArrowShapeData sd{.outerRadius = 30.f,
                                          .innerRadius = 15.f,
                                          .startAngle  = sf::degrees(0.f),
                                          .sweepAngle  = sf::degrees(120.f),
                                          .headLength  = 12.f,
                                          .headWidth   = 18.f};
        const auto                     c  = sd.getCentroid();
        const auto                     lb = sd.getLocalBounds();
        CHECK(c.x >= lb.position.x - 1e-3f);
        CHECK(c.y >= lb.position.y - 1e-3f);
        CHECK(c.x <= lb.position.x + lb.size.x + 1e-3f);
        CHECK(c.y <= lb.position.y + lb.size.y + 1e-3f);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::ArrowShapeData")
{
    SECTION("Local bounds track max(shaftWidth, headWidth)")
    {
        const sf::ArrowShapeData sd{.shaftLength = 50.f, .shaftWidth = 10.f, .headLength = 20.f, .headWidth = 30.f};
        // Local bbox: x in [0, shaftLength + headLength], y centered on the shaft axis.
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, -15.f}, {70.f, 30.f}}));
    }

    SECTION("Global bounds: 90 deg rotation swaps extents")
    {
        const sf::ArrowShapeData sd{.origin      = {35.f, 0.f},
                                    .rotation    = sf::degrees(90.f),
                                    .shaftLength = 50.f,
                                    .shaftWidth  = 10.f,
                                    .headLength  = 20.f,
                                    .headWidth   = 30.f};
        const auto               gb = sd.getGlobalBounds();
        // Width and height swap under 90 deg rotation.
        CHECK(gb.size == Approx(sf::Vec2f{30.f, 70.f}));
    }

    SECTION("Global bounds tighter than transformRect of local bounds at 45 deg")
    {
        // Demonstrates the reason we iterate actual vertices instead of transforming the bbox rect.
        const sf::ArrowShapeData sd{.rotation    = sf::degrees(45.f),
                                    .shaftLength = 50.f,
                                    .shaftWidth  = 10.f,
                                    .headLength  = 20.f,
                                    .headWidth   = 30.f};
        const auto               tight = sd.getGlobalBounds();
        // The naive transformRect AABB of a 70x30 local box rotated 45 deg would be 70.71 wide.
        // The tight bbox must be strictly smaller because the arrow does not fill its bbox corners.
        const float naiveAabbSide = (70.f + 30.f) * 0.70710678f;
        CHECK(tight.size.x < naiveAabbSide);
        CHECK(tight.size.y < naiveAabbSide);
    }

    SECTION("Centroid: Cy is always 0 (symmetric about the shaft axis)")
    {
        const sf::ArrowShapeData sd{.shaftLength = 50.f, .shaftWidth = 10.f, .headLength = 20.f, .headWidth = 30.f};
        CHECK(sd.getCentroid().y == Approx(0.f));
    }

    SECTION("Centroid: shaft-only arrow sits at the shaft midpoint")
    {
        const sf::ArrowShapeData sd{.shaftLength = 50.f, .shaftWidth = 10.f, .headLength = 0.f, .headWidth = 0.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{25.f, 0.f}));
    }

    SECTION("Centroid: head-only arrow sits at the triangle centroid (1/3 in)")
    {
        const sf::ArrowShapeData sd{.shaftLength = 0.f, .shaftWidth = 0.f, .headLength = 30.f, .headWidth = 20.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{10.f, 0.f}));
    }

    SECTION("Centroid: agrees with the 7-vertex polygon shoelace")
    {
        const sf::ArrowShapeData sd{.shaftLength = 50.f, .shaftWidth = 10.f, .headLength = 20.f, .headWidth = 30.f};

        const auto reference = referencePolygonCentroid(7u, [&](const unsigned int i) noexcept {
            return sf::ShapeUtils::computeArrowPoint(i, sd.shaftLength, sd.shaftWidth, sd.headLength, sd.headWidth);
        });

        CHECK(sd.getCentroid() == Approx(reference));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::CrossShapeData")
{
    SECTION("Local bounds match size")
    {
        const sf::CrossShapeData sd{.size = {40.f, 30.f}, .armThickness = 10.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));
    }

    SECTION("Global bounds: 45 deg rotation strictly smaller than transformRect")
    {
        const sf::CrossShapeData sd{.origin       = {20.f, 20.f},
                                    .rotation     = sf::degrees(45.f),
                                    .size         = {40.f, 40.f},
                                    .armThickness = 10.f};
        const auto               tight         = sd.getGlobalBounds();
        const float              naiveAabbSide = 40.f * 1.41421356f;
        CHECK(tight.size.x < naiveAabbSide);
    }

    SECTION("Centroid: coincides with cross center (symmetric)")
    {
        const sf::CrossShapeData sd{.size = {40.f, 30.f}, .armThickness = 10.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 15.f}));
    }

    SECTION("Centroid: agrees with the 12-vertex polygon shoelace")
    {
        const sf::CrossShapeData sd{.size = {40.f, 30.f}, .armThickness = 10.f};
        const auto               reference = referencePolygonCentroid(12u, [&](const unsigned int i) noexcept {
            return sf::ShapeUtils::computeCrossPoint(i, sd.size, sd.armThickness);
        });
        CHECK(sd.getCentroid() == Approx(reference));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::ChevronShapeData")
{
    SECTION("Local bounds match size")
    {
        const sf::ChevronShapeData sd{.size = {40.f, 30.f}, .thickness = 8.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));
    }

    SECTION("Degenerate size.y yields an empty bounding box (nothing drawn)")
    {
        const sf::ChevronShapeData sd{.position = {5.f, 5.f}, .size = {40.f, 0.f}, .thickness = 0.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {}));
    }

    SECTION("Thickness beyond size.y/2 cleanly falls back to a filled triangle")
    {
        // At exactly `size.y/2`, the inner tip collapses to `(0, size.y/2)`; beyond that,
        // the shape clamps to the same degenerate triangle rather than self-intersecting.
        const sf::ChevronShapeData full{.size = {40.f, 30.f}, .thickness = 15.f};
        const sf::ChevronShapeData over{.size = {40.f, 30.f}, .thickness = 1000.f};

        // Both should produce the outer-triangle AABB, which spans the full local size.
        CHECK(rectsApproxEqual(full.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));
        CHECK(rectsApproxEqual(over.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));

        // Tight global bounds should be identical for any thickness >= size.y/2.
        CHECK(rectsApproxEqual(full.getGlobalBounds(), over.getGlobalBounds()));
    }

    SECTION("Centroid: Cy is always size.y / 2 (symmetric about the horizontal axis)")
    {
        const sf::ChevronShapeData sd{.size = {40.f, 30.f}, .thickness = 8.f};
        CHECK(sd.getCentroid().y == Approx(15.f));
    }

    SECTION("Centroid: thickness exactly size.y/2 degenerates to a triangle centroid")
    {
        const sf::ChevronShapeData sd{.size = {40.f, 30.f}, .thickness = 15.f};
        // Triangle (0,0), (40,15), (0,30) has centroid ((0+40+0)/3, (0+15+30)/3) = (40/3, 15).
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{40.f / 3.f, 15.f}));
    }

    SECTION("Centroid: agrees with the 6-vertex polygon shoelace for a thin chevron")
    {
        const sf::ChevronShapeData sd{.size = {40.f, 30.f}, .thickness = 8.f};
        const auto                 reference = referencePolygonCentroid(6u, [&](const unsigned int i) noexcept {
            return sf::ShapeUtils::computeChevronPoint(i, sd.size, sd.thickness);
        });
        CHECK(sd.getCentroid() == Approx(reference));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::TrapezoidShapeData")
{
    SECTION("Local bounds use max(topWidth, bottomWidth)")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 20.f, .bottomWidth = 40.f, .height = 30.f};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));
    }

    SECTION("Rectangular special case: equal widths")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 20.f, .bottomWidth = 20.f, .height = 10.f};
        // Degenerates into a rectangle; global bounds should match local since trapezoid corners are on the bbox.
        CHECK(rectsApproxEqual(sd.getGlobalBounds(), {{0.f, 0.f}, {20.f, 10.f}}));
    }

    SECTION("Centroid: rectangular case sits at the rectangle center")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 20.f, .bottomWidth = 20.f, .height = 10.f};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{10.f, 5.f}));
    }

    SECTION("Centroid: agrees with the classic (a + 2b)/(3(a+b)) formula")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 20.f, .bottomWidth = 40.f, .height = 30.f};
        // Cx at bbox center (max width = 40), Cy = 30 * (20 + 80) / (3 * 60) = 30 * 100 / 180 = 16.666...
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 30.f * 100.f / 180.f}));
    }

    SECTION("Centroid: agrees with the 4-vertex polygon shoelace")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 20.f, .bottomWidth = 40.f, .height = 30.f};
        const auto                   reference = referencePolygonCentroid(4u, [&](const unsigned int i) noexcept {
            return sf::ShapeUtils::computeTrapezoidPoint(i, sd.topWidth, sd.bottomWidth, sd.height);
        });
        CHECK(sd.getCentroid() == Approx(reference));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] sf::HeartShapeData")
{
    SECTION("Local bounds match size")
    {
        const sf::HeartShapeData sd{.size = {40.f, 30.f}, .pointCount = 64u};
        CHECK(rectsApproxEqual(sd.getLocalBounds(), {{0.f, 0.f}, {40.f, 30.f}}));
    }

    SECTION("Global bounds fit within local bounds at identity transform")
    {
        const sf::HeartShapeData sd{.size = {40.f, 30.f}, .pointCount = 128u};
        const auto               gb = sd.getGlobalBounds();
        // With denser tessellation, global bounds should closely match local bounds.
        CHECK(gb.position.x >= -0.01f);
        CHECK(gb.position.y >= -0.01f);
        CHECK(gb.position.x + gb.size.x <= 40.01f);
        CHECK(gb.position.y + gb.size.y <= 30.01f);
    }

    SECTION("Centroid: Cx lies on the axis of symmetry")
    {
        const sf::HeartShapeData sd{.size = {40.f, 30.f}, .pointCount = 64u};
        CHECK(sd.getCentroid().x == Approx(20.f));
    }

    SECTION("Centroid: lies inside the heart's bounding box")
    {
        const sf::HeartShapeData sd{.size = {40.f, 30.f}, .pointCount = 64u};
        const auto               c = sd.getCentroid();
        CHECK(c.x >= 0.f);
        CHECK(c.y >= 0.f);
        CHECK(c.x <= 40.f);
        CHECK(c.y <= 30.f);
    }

    SECTION("Centroid: degenerate pointCount falls back to bbox center")
    {
        const sf::HeartShapeData sd{.size = {40.f, 30.f}, .pointCount = 3u};
        CHECK(sd.getCentroid() == Approx(sf::Vec2f{20.f, 15.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] Centroid membership: convex shapes contain their centroid")
{
    // A centroid is not guaranteed to lie inside a non-convex polygon in general, but for the
    // convex / centrally-symmetric shapes in the library it should.
    SECTION("Circle")
    {
        const sf::CircleShapeData sd{.radius = 10.f};
        const auto                c  = sd.getCentroid();
        const auto                lb = sd.getLocalBounds();
        CHECK(lb.contains(c));
    }

    SECTION("Ellipse")
    {
        const sf::EllipseShapeData sd{.horizontalRadius = 20.f, .verticalRadius = 10.f};
        CHECK(sd.getLocalBounds().contains(sd.getCentroid()));
    }

    SECTION("Rectangle")
    {
        const sf::RectangleShapeData sd{.size = {40.f, 20.f}};
        CHECK(sd.getLocalBounds().contains(sd.getCentroid()));
    }

    SECTION("Trapezoid")
    {
        const sf::TrapezoidShapeData sd{.topWidth = 10.f, .bottomWidth = 40.f, .height = 30.f};
        CHECK(sd.getLocalBounds().contains(sd.getCentroid()));
    }

    SECTION("Arrow")
    {
        const sf::ArrowShapeData sd{.shaftLength = 50.f, .shaftWidth = 10.f, .headLength = 20.f, .headWidth = 30.f};
        CHECK(sd.getLocalBounds().contains(sd.getCentroid()));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] Anchor-point mixins")
{
    SECTION("Local helpers read from getLocalBounds")
    {
        const sf::RectangleShapeData sd{.size = {40.f, 20.f}};
        CHECK(sd.getLocalCenter() == Approx(sf::Vec2f{20.f, 10.f}));
        CHECK(sd.getLocalTopLeft() == Approx(sf::Vec2f{0.f, 0.f}));
        CHECK(sd.getLocalBottomRight() == Approx(sf::Vec2f{40.f, 20.f}));
        CHECK(sd.getLocalWidth() == Approx(40.f));
        CHECK(sd.getLocalHeight() == Approx(20.f));
    }

    SECTION("Global helpers read from getGlobalBounds")
    {
        const sf::RectangleShapeData sd{.position = {5.f, 5.f}, .size = {40.f, 20.f}};
        CHECK(sd.getGlobalTopLeft() == Approx(sf::Vec2f{5.f, 5.f}));
        CHECK(sd.getGlobalCenter() == Approx(sf::Vec2f{25.f, 15.f}));
        CHECK(sd.getGlobalBottomRight() == Approx(sf::Vec2f{45.f, 25.f}));
    }

    SECTION("setGlobalCenter repositions the shape")
    {
        sf::RectangleShapeData sd{.size = {40.f, 20.f}};
        sd.setGlobalCenter({100.f, 50.f});
        CHECK(sd.getGlobalCenter() == Approx(sf::Vec2f{100.f, 50.f}));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] Global bounds invariance under rotation about the geometric center")
{
    // For any shape whose origin is placed at its geometric (local) center and whose local
    // bounds are square, rotating by 90 deg produces the same AABB size. This is a direct
    // sanity check that our bounds formulas handle rotation correctly.
    SECTION("Circle")
    {
        const sf::CircleShapeData rot0{.origin = {10.f, 10.f}, .radius = 10.f};
        const sf::CircleShapeData rot90{.origin = {10.f, 10.f}, .rotation = sf::degrees(90.f), .radius = 10.f};
        CHECK(rot0.getGlobalBounds().size == Approx(rot90.getGlobalBounds().size));
    }

    SECTION("Ring")
    {
        const sf::RingShapeData rot0{.origin = {20.f, 20.f}, .outerRadius = 20.f, .innerRadius = 10.f};
        const sf::RingShapeData rot90{.origin      = {20.f, 20.f},
                                      .rotation    = sf::degrees(90.f),
                                      .outerRadius = 20.f,
                                      .innerRadius = 10.f};
        CHECK(rot0.getGlobalBounds().size == Approx(rot90.getGlobalBounds().size));
    }

    SECTION("Square Rectangle")
    {
        const sf::RectangleShapeData rot0{.origin = {10.f, 10.f}, .size = {20.f, 20.f}};
        const sf::RectangleShapeData rot90{.origin = {10.f, 10.f}, .rotation = sf::degrees(90.f), .size = {20.f, 20.f}};
        CHECK(rot0.getGlobalBounds().size == Approx(rot90.getGlobalBounds().size));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Graphics] hasVisibleGeometry")
{
    // Default-constructed shape data should not report anything drawable: every type's
    // "zero state" (radius = 0, size = {0, 0}, etc.) leaves the draw path with no geometry.
    SECTION("Default-constructed shapes have no visible geometry")
    {
        CHECK_FALSE(sf::ArrowShapeData{.shaftLength = 0.f, .shaftWidth = 0.f, .headLength = 0.f, .headWidth = 0.f}
                        .hasVisibleGeometry());
        CHECK_FALSE(sf::ChevronShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::CircleShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::CogShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::CrossShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::CurvedArrowShapeData{.outerRadius = 0.f, .innerRadius = 0.f}.hasVisibleGeometry());
        CHECK_FALSE(sf::EllipseShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::HeartShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::PieSliceShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::RectangleShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::RingPieSliceShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::RingShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::RoundedRectangleShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::StarShapeData{}.hasVisibleGeometry());
        CHECK_FALSE(sf::TrapezoidShapeData{}.hasVisibleGeometry());
    }

    // Reasonable "minimally valid" configurations should be visible.
    SECTION("Minimally valid configurations are visible")
    {
        CHECK(sf::ArrowShapeData{}.hasVisibleGeometry()); // defaults are all positive
        CHECK(sf::ChevronShapeData{.size = {40.f, 30.f}, .thickness = 5.f}.hasVisibleGeometry());
        CHECK(sf::CircleShapeData{.radius = 10.f}.hasVisibleGeometry());
        CHECK(sf::CogShapeData{.outerRadius = 20.f, .innerRadius = 10.f}.hasVisibleGeometry());
        CHECK(sf::CrossShapeData{.size = {40.f, 30.f}, .armThickness = 10.f}.hasVisibleGeometry());
        CHECK(sf::CurvedArrowShapeData{.outerRadius = 30.f, .innerRadius = 15.f}.hasVisibleGeometry());
        CHECK(sf::EllipseShapeData{.horizontalRadius = 20.f, .verticalRadius = 10.f}.hasVisibleGeometry());
        CHECK(sf::HeartShapeData{.size = {40.f, 30.f}}.hasVisibleGeometry());
        CHECK(sf::PieSliceShapeData{.radius = 10.f, .sweepAngle = sf::degrees(90.f)}.hasVisibleGeometry());
        CHECK(sf::RectangleShapeData{.size = {40.f, 20.f}}.hasVisibleGeometry());
        CHECK(sf::RingPieSliceShapeData{.outerRadius = 30.f, .innerRadius = 15.f, .sweepAngle = sf::degrees(90.f)}
                  .hasVisibleGeometry());
        CHECK(sf::RingShapeData{.outerRadius = 30.f, .innerRadius = 15.f}.hasVisibleGeometry());
        CHECK(sf::RoundedRectangleShapeData{.size = {40.f, 30.f}, .cornerRadius = 5.f}.hasVisibleGeometry());
        CHECK(sf::StarShapeData{.outerRadius = 30.f, .innerRadius = 15.f}.hasVisibleGeometry());
        CHECK(sf::TrapezoidShapeData{.topWidth = 20.f, .bottomWidth = 40.f, .height = 30.f}.hasVisibleGeometry());
    }

    // Per-shape boundary conditions: each knob that can falsify the predicate.
    SECTION("ArrowShapeData visibility boundaries")
    {
        // Zero total length -> invisible.
        CHECK_FALSE(sf::ArrowShapeData{.shaftLength = 0.f, .headLength = 0.f}.hasVisibleGeometry());
        // Zero thickness on both the shaft and the head -> invisible.
        CHECK_FALSE(sf::ArrowShapeData{.shaftWidth = 0.f, .headWidth = 0.f}.hasVisibleGeometry());
        // Negative dimension -> invisible.
        CHECK_FALSE(sf::ArrowShapeData{.shaftLength = -1.f}.hasVisibleGeometry());
    }

    SECTION("CircleShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::CircleShapeData{.radius = 0.f}.hasVisibleGeometry());
        CHECK_FALSE(sf::CircleShapeData{.radius = 10.f, .pointCount = 2u}.hasVisibleGeometry());
    }

    SECTION("CogShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::CogShapeData{.outerRadius = 20.f, .innerRadius = 10.f, .toothCount = 2u}.hasVisibleGeometry());
        CHECK_FALSE(sf::CogShapeData{.outerRadius = 20.f, .innerRadius = 10.f, .toothCount = 8u, .toothWidthRatio = 0.f}
                        .hasVisibleGeometry());
        CHECK_FALSE(sf::CogShapeData{.outerRadius = 20.f, .innerRadius = 10.f, .toothCount = 8u, .toothWidthRatio = 1.f}
                        .hasVisibleGeometry());
    }

    SECTION("CrossShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::CrossShapeData{.size = {0.f, 30.f}}.hasVisibleGeometry());
        CHECK_FALSE(sf::CrossShapeData{.size = {40.f, 0.f}}.hasVisibleGeometry());
        CHECK_FALSE(sf::CrossShapeData{.size = {40.f, 30.f}, .armThickness = -1.f}.hasVisibleGeometry());
    }

    SECTION("EllipseShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::EllipseShapeData{.horizontalRadius = 0.f, .verticalRadius = 10.f}.hasVisibleGeometry());
        CHECK_FALSE(sf::EllipseShapeData{.horizontalRadius = 10.f, .verticalRadius = 0.f}.hasVisibleGeometry());
        CHECK_FALSE(
            sf::EllipseShapeData{.horizontalRadius = 10.f, .verticalRadius = 10.f, .pointCount = 2u}.hasVisibleGeometry());
    }

    SECTION("HeartShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::HeartShapeData{.size = {40.f, 30.f}, .pointCount = 2u}.hasVisibleGeometry());
    }

    SECTION("RectangleShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::RectangleShapeData{.size = {0.f, 20.f}}.hasVisibleGeometry());
        CHECK_FALSE(sf::RectangleShapeData{.size = {40.f, 0.f}}.hasVisibleGeometry());
    }

    SECTION("RoundedRectangleShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::RoundedRectangleShapeData{.size = {40.f, 30.f}, .cornerPointCount = 0u}.hasVisibleGeometry());
        CHECK_FALSE(sf::RoundedRectangleShapeData{.size = {40.f, 30.f}, .cornerRadius = -1.f}.hasVisibleGeometry());
    }

    SECTION("StarShapeData visibility boundaries")
    {
        CHECK_FALSE(sf::StarShapeData{.outerRadius = 30.f, .pointCount = 1u}.hasVisibleGeometry());
        CHECK_FALSE(sf::StarShapeData{.outerRadius = 30.f, .innerRadius = -1.f}.hasVisibleGeometry());
    }

    SECTION("TrapezoidShapeData visibility boundaries")
    {
        // Zero height -> no area -> invisible.
        CHECK_FALSE(sf::TrapezoidShapeData{.topWidth = 20.f, .bottomWidth = 40.f, .height = 0.f}.hasVisibleGeometry());
        // Zero on both widths -> no area -> invisible.
        CHECK_FALSE(sf::TrapezoidShapeData{.topWidth = 0.f, .bottomWidth = 0.f, .height = 30.f}.hasVisibleGeometry());
        // One zero is fine (degenerates to a triangle).
        CHECK(sf::TrapezoidShapeData{.topWidth = 0.f, .bottomWidth = 40.f, .height = 30.f}.hasVisibleGeometry());
    }

    // When `hasVisibleGeometry()` is false, the bounds should be empty.
    SECTION("Invisible shapes produce empty bounds")
    {
        const sf::ArrowShapeData invalidArrow{.shaftLength = 0.f, .shaftWidth = 0.f, .headLength = 0.f, .headWidth = 0.f};
        const sf::CircleShapeData           invalidCircle{.radius = 0.f};
        const sf::CrossShapeData            invalidCross{.size = {0.f, 0.f}};
        const sf::EllipseShapeData          invalidEllipse{.horizontalRadius = 0.f, .verticalRadius = 0.f};
        const sf::HeartShapeData            invalidHeart{.size = {0.f, 0.f}};
        const sf::RectangleShapeData        invalidRect{.size = {0.f, 0.f}};
        const sf::RoundedRectangleShapeData invalidRoundedRect{.size = {0.f, 0.f}};
        const sf::StarShapeData             invalidStar{.outerRadius = 0.f};
        const sf::TrapezoidShapeData        invalidTrap{.topWidth = 0.f, .bottomWidth = 0.f, .height = 30.f};
        const sf::CogShapeData              invalidCog{.outerRadius = 0.f};

        CHECK(rectsApproxEqual(invalidArrow.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidArrow.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCircle.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCircle.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCross.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCross.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidEllipse.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidEllipse.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidHeart.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidHeart.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidRect.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidRect.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidRoundedRect.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidRoundedRect.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidStar.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidStar.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidTrap.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidTrap.getGlobalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCog.getLocalBounds(), {}));
        CHECK(rectsApproxEqual(invalidCog.getGlobalBounds(), {}));
    }
}
