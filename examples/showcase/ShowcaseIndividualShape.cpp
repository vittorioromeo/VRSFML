#include "ShowcaseExample.hpp"
#include "ShowcaseIndividualShape.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"

#include <imgui.h>


////////////////////////////////////////////////////////////
decltype(auto) ExampleIndividualShape::callWithActiveShape(auto&& f)
{
    switch (m_shapeIndex)
    {
        case 0:
            return f(m_sdArrow);
        case 1:
            return f(m_sdCircle);
        case 2:
            return f(m_sdCurvedArrow);
        case 3:
            return f(m_sdEllipse);
        case 4:
            return f(m_sdPieSlice);
        case 5:
            return f(m_sdRectangle);
        case 6:
            return f(m_sdRingShape);
        case 7:
            return f(m_sdRingPieSlice);
        case 8:
            return f(m_sdRoundedRectangle);
    }

    SFML_BASE_ASSERT(m_shapeIndex == 9);
    return f(m_sdStar);
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
    ImGui::Begin("Shape Playground", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    constexpr const char* shapeNames[]{
        "Arrow",
        "Circle",
        "CurvedArrow",
        "Ellipse",
        "PieSlice",
        "Rectangle",
        "Ring",
        "RingPieSlice",
        "RoundedRectangle",
        "Star",
    };

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
    SLIDERFLOAT(*this, m_outlineThickness, 0.f, 50.f);
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
            SLIDERFLOAT(m_sdCircle, radius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCircle, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdCircle, pointCount, 3u, 100u);
            break;
        case 2:
            SLIDERFLOAT(m_sdCurvedArrow, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdCurvedArrow, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdCurvedArrow, headLength, 0.f, 100.f);
            SLIDERFLOAT(m_sdCurvedArrow, headWidth, 0.f, 100.f);
            SLIDERUINT(m_sdCurvedArrow, pointCount, 3u, 100u);
            break;
        case 3:
            SLIDERFLOAT(m_sdEllipse, horizontalRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdEllipse, verticalRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdEllipse, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdEllipse, pointCount, 3u, 100u);
            break;
        case 4:
            SLIDERFLOAT(m_sdPieSlice, radius, 0.f, 100.f);
            SLIDERFLOAT(m_sdPieSlice, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdPieSlice, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdPieSlice, pointCount, 3u, 100u);
            break;
        case 5:
            SLIDERFLOAT(m_sdRectangle, size.x, 0.f, 100.f);
            SLIDERFLOAT(m_sdRectangle, size.y, 0.f, 100.f);
            break;
        case 6:
            SLIDERFLOAT(m_sdRingShape, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingShape, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingShape, startAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdRingShape, pointCount, 3u, 100u);
            break;
        case 7:
            SLIDERFLOAT(m_sdRingPieSlice, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingPieSlice, innerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdRingPieSlice, startAngle.radians, 0.f, sf::base::tau);
            SLIDERFLOAT(m_sdRingPieSlice, sweepAngle.radians, 0.f, sf::base::tau);
            SLIDERUINT(m_sdRingPieSlice, pointCount, 3u, 100u);
            break;
        case 8:
            SLIDERFLOAT(m_sdRoundedRectangle, size.x, 0.f, 100.f);
            SLIDERFLOAT(m_sdRoundedRectangle, size.y, 0.f, 100.f);
            SLIDERFLOAT(m_sdRoundedRectangle, cornerRadius, 0.f, 100.f);
            SLIDERUINT(m_sdRoundedRectangle, cornerPointCount, 3u, 100u);
            break;
        case 9:
            SLIDERFLOAT(m_sdStar, outerRadius, 0.f, 100.f);
            SLIDERFLOAT(m_sdStar, innerRadius, 0.f, 100.f);
            SLIDERUINT(m_sdStar, pointCount, 3u, 100u);
            break;
    }

    ImGui::End();
}


////////////////////////////////////////////////////////////
void ExampleIndividualShape::draw()
{
    m_phase = 0.f;

    callWithActiveShape([this](auto& shapeData) { m_deps.rtGame->draw(shapeData, {.view = *m_deps.view}); });
}
