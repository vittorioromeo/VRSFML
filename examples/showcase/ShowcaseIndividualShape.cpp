#include "ShowcaseExample.hpp"
#include "ShowcaseIndividualShape.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr const char* shapeNames[]{
    "Arrow",
    "Chevron",
    "Circle",
    "Cog",
    "Cross",
    "CurvedArrow",
    "Ellipse",
    "Heart",
    "PieSlice",
    "Rectangle",
    "Ring",
    "RingPieSlice",
    "RoundedRectangle",
    "Star",
    "Trapezoid",
};

} // namespace


////////////////////////////////////////////////////////////
decltype(auto) ExampleIndividualShape::callWithActiveShape(auto&& f)
{
    // clang-format off
    switch (m_shapeIndex)
    {
        case 0:  return f(m_sdArrow);
        case 1:  return f(m_sdChevron);
        case 2:  return f(m_sdCircle);
        case 3:  return f(m_sdCog);
        case 4:  return f(m_sdCross);
        case 5:  return f(m_sdCurvedArrow);
        case 6:  return f(m_sdEllipse);
        case 7:  return f(m_sdHeart);
        case 8:  return f(m_sdPieSlice);
        case 9:  return f(m_sdRectangle);
        case 10: return f(m_sdRingShape);
        case 11: return f(m_sdRingPieSlice);
        case 12: return f(m_sdRoundedRectangle);
        case 13: return f(m_sdStar);
        case 14: return f(m_sdTrapezoid);
    }
    // clang-format on

    SFML_BASE_ASSERT(false);
    sf::base::abort();
}


////////////////////////////////////////////////////////////
ExampleIndividualShape::ExampleIndividualShape(const GameDependencies& deps) :
    ShowcaseExample{"IndividualShape"},
    m_deps{deps}
{
}


////////////////////////////////////////////////////////////
void ExampleIndividualShape::update(const float deltaTimeMs)
{
    m_time += deltaTimeMs;

    callWithActiveShape([this](auto& shapeData)
    {
        const auto fillColor = sf::Color::Red.withRotatedHue(m_time + m_phase * 65.f);

        shapeData.fillColor        = fillColor;
        shapeData.outlineColor     = fillColor.withRotatedHue(180.f);
        shapeData.outlineThickness = m_outlineThickness;
        shapeData.miterLimit       = m_miterLimit;

        shapeData.position = m_position;
        shapeData.origin   = m_origin;
        shapeData.rotation = m_rotation;
        shapeData.scale    = m_scale;
    });
}


////////////////////////////////////////////////////////////
void ExampleIndividualShape::imgui()
{
    // Mouse-driven interaction. This has to run from `imgui()` rather than `update()` because
    // `io.MouseWheel` and `io.MouseDelta` are only populated by `ImGui::NewFrame()`, which the
    // showcase driver invokes between the example's `update()` and `imgui()` calls -- so both
    // fields are stale when read from `update()`. Gated on `!WantCaptureMouse` so the sliders
    // in the Shape Playground window keep working when hovered.
    const auto& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            m_position.x += io.MouseDelta.x;
            m_position.y += io.MouseDelta.y;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            m_rotation += sf::degrees(io.MouseDelta.x * 0.5f);

        if (io.MouseWheel != 0.f)
        {
            const float factor = 1.f + io.MouseWheel * 0.1f;
            m_scale.x *= factor;
            m_scale.y *= factor;
        }
    }

    ImGui::Begin("Shape Playground", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Show position/origin", &m_showPositionOrigin);
    ImGui::SameLine();
    ImGui::Checkbox("Show bounding box", &m_showBoundingBox);
    ImGui::SameLine();
    ImGui::Checkbox("Show centroid", &m_showCentroid);

    if (ImGui::Button("Set origin to center"))
        callWithActiveShape([this](auto& shapeData) { m_origin = shapeData.getLocalBounds().getCenter(); });

    ImGui::SameLine();

    if (ImGui::Button("Set origin to centroid"))
        callWithActiveShape([this](auto& shapeData) { m_origin = shapeData.getCentroid(); });

    ImGui::TextUnformatted("Drag LMB to move, drag RMB horizontally to rotate, mouse wheel to scale.");

    ImGui::SetNextItemWidth(120.f);
    ImGui::Combo("Shape", &m_shapeIndex, shapeNames, sf::base::getArraySize(shapeNames));

#define SLIDERFLOAT(obj, member, min, max) \
    ImGui::SliderFloat(#member "##" #obj, &(obj).member, min, max, "%.3f", ImGuiSliderFlags_NoRoundToFormat)

#define SLIDERUINT(obj, member, min, max) \
    ImGui::SliderInt(#member "##" #obj, reinterpret_cast<int*>(&(obj).member), min, max)

    SLIDERFLOAT(*this, m_position.x, -128.f, resolution.x);
    SLIDERFLOAT(*this, m_position.y, -128.f, resolution.y);
    SLIDERFLOAT(*this, m_origin.x, -256.f, 256.f);
    SLIDERFLOAT(*this, m_origin.y, -256.f, 256.f);
    SLIDERFLOAT(*this, m_rotation.radians, 0.f, sf::base::tau);
    SLIDERFLOAT(*this, m_scale.x, 0.f, 10.f);
    SLIDERFLOAT(*this, m_scale.y, 0.f, 10.f);
    SLIDERFLOAT(*this, m_outlineThickness, -50.f, 50.f);
    SLIDERFLOAT(*this, m_miterLimit, 1.f, 10.f);

    switch (m_shapeIndex)
    {
        case 0:
            SLIDERFLOAT(m_sdArrow, shaftLength, 0.f, 100.f);
            SLIDERFLOAT(m_sdArrow, shaftWidth, 0.f, 100.f);
            SLIDERFLOAT(m_sdArrow, headLength, 0.f, 100.f);
            SLIDERFLOAT(m_sdArrow, headWidth, 0.f, 100.f);
            break;

        case 1:
            SLIDERFLOAT(m_sdChevron, size.x, 0.f, 200.f);
            SLIDERFLOAT(m_sdChevron, size.y, 1.f, 200.f);
            SLIDERFLOAT(m_sdChevron, thickness, 0.f, 100.f);
            break;

        case 2:
            SLIDERFLOAT(m_sdCircle, radius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCircle, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdCircle, pointCount, 3u, 100u);
            break;

        case 3:
            SLIDERFLOAT(m_sdCog, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCog, innerRadius, 0.f, 100.f);
            SLIDERUINT(m_sdCog, toothCount, 3u, 32u);
            SLIDERFLOAT(m_sdCog, toothWidthRatio, 0.05f, 0.95f);
            break;

        case 4:
            SLIDERFLOAT(m_sdCross, size.x, 0.f, 200.f);
            SLIDERFLOAT(m_sdCross, size.y, 0.f, 200.f);
            SLIDERFLOAT(m_sdCross, armThickness, 0.f, 100.f);
            break;

        case 5:
            SLIDERFLOAT(m_sdCurvedArrow, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdCurvedArrow, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdCurvedArrow, headLength, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, headWidth, 0.f, 100.f);
            SLIDERUINT(m_sdCurvedArrow, pointCount, 3u, 100u);
            break;

        case 6:
            SLIDERFLOAT(m_sdEllipse, horizontalRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdEllipse, verticalRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdEllipse, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdEllipse, pointCount, 3u, 100u);
            break;

        case 7:
            SLIDERFLOAT(m_sdHeart, size.x, 0.f, 200.f);
            SLIDERFLOAT(m_sdHeart, size.y, 0.f, 200.f);
            SLIDERUINT(m_sdHeart, pointCount, 8u, 256u);
            break;

        case 8:
            SLIDERFLOAT(m_sdPieSlice, radius, 0.f, 100.f);
            SLIDERFLOAT(m_sdPieSlice, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdPieSlice, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdPieSlice, pointCount, 3u, 100u);
            break;

        case 9:
            SLIDERFLOAT(m_sdRectangle, size.x, 0.f, 100.f);
            SLIDERFLOAT(m_sdRectangle, size.y, 0.f, 100.f);
            break;

        case 10:
            SLIDERFLOAT(m_sdRingShape, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingShape, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingShape, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdRingShape, pointCount, 3u, 100u);
            break;

        case 11:
            SLIDERFLOAT(m_sdRingPieSlice, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingPieSlice, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingPieSlice, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdRingPieSlice, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdRingPieSlice, pointCount, 3u, 100u);
            break;

        case 12:
            SLIDERFLOAT(m_sdRoundedRectangle, size.x, 0.f, 100.f);
            SLIDERFLOAT(m_sdRoundedRectangle, size.y, 0.f, 100.f);
            SLIDERFLOAT(m_sdRoundedRectangle, cornerRadius, 0.f, 100.f);
            SLIDERUINT(m_sdRoundedRectangle, cornerPointCount, 3u, 100u);
            break;

        case 13:
            SLIDERFLOAT(m_sdStar, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdStar, innerRadius, 0.f, 100.f);
            SLIDERUINT(m_sdStar, pointCount, 3u, 100u);
            break;

        case 14:
            SLIDERFLOAT(m_sdTrapezoid, topWidth, 0.f, 200.f);
            SLIDERFLOAT(m_sdTrapezoid, bottomWidth, 0.f, 200.f);
            SLIDERFLOAT(m_sdTrapezoid, height, 0.f, 200.f);
            break;
    }

    ImGui::End();
}


////////////////////////////////////////////////////////////
void ExampleIndividualShape::draw()
{
    m_phase = 0.f;

    callWithActiveShape([this](auto& shapeData)
    {
        m_deps.rtGame->draw(shapeData, {.view = *m_deps.view});

        if (m_showPositionOrigin)
        {
            m_deps.rtGame->draw(
                sf::CircleShapeData{
                    .position         = sf::Transform::fromPositionScaleOriginRotation(shapeData.position,
                                                                                       shapeData.scale,
                                                                                       shapeData.origin,
                                                                                       shapeData.rotation)
                                            .transformPoint(shapeData.origin),
                    .origin           = {2.f, 2.f},
                    .fillColor        = sf::Color::Magenta,
                    .outlineColor     = sf::Color::Cyan,
                    .outlineThickness = 1.f,
                    .radius           = 2.f,
                },
                {.view = *m_deps.view});
        }

        if (m_showBoundingBox)
        {
            const auto globalBounds = shapeData.getGlobalBounds();

            m_deps.rtGame->draw(
                sf::RectangleShapeData{
                    .position         = globalBounds.position,
                    .origin           = {0.f, 0.f},
                    .fillColor        = sf::Color::Transparent,
                    .outlineColor     = sf::Color::Green,
                    .outlineThickness = 1.f,
                    .size             = globalBounds.size,
                },
                {.view = *m_deps.view});
        }

        if (m_showCentroid)
        {
            const auto transform = sf::Transform::fromPositionScaleOriginRotation(shapeData.position,
                                                                                  shapeData.scale,
                                                                                  shapeData.origin,
                                                                                  shapeData.rotation);

            m_deps.rtGame->draw(
                sf::CircleShapeData{
                    .position         = transform.transformPoint(shapeData.getCentroid()),
                    .origin           = {2.f, 2.f},
                    .fillColor        = sf::Color::Yellow,
                    .outlineColor     = sf::Color::Black,
                    .outlineThickness = 1.f,
                    .radius           = 2.f,
                },
                {.view = *m_deps.view});
        }
    });
}
