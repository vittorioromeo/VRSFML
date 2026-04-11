#include "ShowcaseExample.hpp"
#include "ShowcaseShapes.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Math/Sin.hpp"

#include <initializer_list>


////////////////////////////////////////////////////////////
auto ExampleShapes::applyCommonSettings(const sf::Vec2f currentOffset, auto shapeData)
{
    const auto fillColor = sf::Color::Red.withRotatedHue(m_time + m_phase * 65.f);

    shapeData.position += currentOffset;
    shapeData.position += {64.f, 64.f};

    shapeData.fillColor        = fillColor;
    shapeData.outlineColor     = fillColor.withRotatedHue(180.f);
    shapeData.outlineThickness = sf::base::fabs(4.f * sf::base::sin(m_time * 0.05f + m_phase));

    shapeData.rotation = sf::degrees(sf::base::fmod(m_time * 1.f + m_phase * 45.f, 360.f));

    shapeData.textureRect = {
        .position = {0.f, 0.f},
        .size     = {1.f, 1.f},
    };

    return shapeData;
}


////////////////////////////////////////////////////////////
sf::VertexSpan ExampleShapes::drawShape(const sf::Vec2f currentOffset, const char* label, const auto& shapeData)
{
    auto result = m_deps.rtGame->draw(applyCommonSettings(currentOffset, shapeData), {.view = *m_deps.view});

    m_deps.rtGame->draw(*m_deps.font,
                        sf::TextData{
                            .position         = shapeData.position + currentOffset,
                            .string           = label,
                            .characterSize    = 16,
                            .outlineColor     = sf::Color::Black,
                            .outlineThickness = 2.f,
                        },
                        {.view = *m_deps.view});

    m_phase += 0.1f;

    return result;
}


////////////////////////////////////////////////////////////
float ExampleShapes::getPhasedValue(const float timeMultiplier, const float phaseMultiplier) const
{
    return sf::base::fabs(sf::base::sin(m_time * timeMultiplier + m_phase * phaseMultiplier));
}


////////////////////////////////////////////////////////////
void ExampleShapes::drawAllShapes(const sf::Vec2f offset)
{
    drawShape(offset,
              "Circle",
              sf::CircleShapeData{
                  .position   = {32.f, 32.f},
                  .origin     = {64.f, 64.f},
                  .radius     = 64.f,
                  .pointCount = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.04f, 2.f)),
              });

    drawShape(offset,
              "Ellipse",
              sf::EllipseShapeData{
                  .position         = {196.f, 32.f},
                  .origin           = {64.f, 32.f},
                  .horizontalRadius = 64.f,
                  .verticalRadius   = 32.f,
                  .pointCount       = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.06f, 3.5f)),
              });

    drawShape(offset,
              "PieSlice",
              sf::PieSliceShapeData{
                  .position   = {364.f, 32.f},
                  .origin     = {64.f, 64.f},
                  .radius     = 64.f,
                  .startAngle = sf::degrees(0.f),
                  .sweepAngle = sf::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                  .pointCount = 32u,
              });

    drawShape(offset,
              "Arrow",
              sf::ArrowShapeData{
                  .position    = {32.f, 196.f},
                  .origin      = {(64.f + 48.f) / 2.f, 0.f},
                  .shaftLength = 64.f,
                  .shaftWidth  = 32.f + (32.f * getPhasedValue(0.04f, 2.f)),
                  .headLength  = 48.f,
                  .headWidth   = 96.f - (64.f * getPhasedValue(0.06f, 3.f)),
              });

    drawShape(offset,
              "RoundedRectangle",
              sf::RoundedRectangleShapeData{
                  .position         = {196.f, 196.f},
                  .origin           = {64.f, 32.f},
                  .size             = {128.f, 64.f},
                  .cornerRadius     = 3.f + (29.f * getPhasedValue(0.1f, 1.5f)),
                  .cornerPointCount = 16u,
              });

    drawShape(offset,
              "RingPieSlice",
              sf::RingPieSliceShapeData{
                  .position    = {364.f, 196.f},
                  .origin      = {64.f, 64.f},
                  .outerRadius = 64.f,
                  .innerRadius = 32.f + (16.f * getPhasedValue(0.2f, 0.75f)),
                  .startAngle  = sf::degrees(0.f),
                  .sweepAngle  = sf::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                  .pointCount  = 32u,
              });

    drawShape(offset,
              "Ring",
              sf::RingShapeData{
                  .position    = {364.f, 364.f},
                  .origin      = {64.f, 64.f},
                  .outerRadius = 64.f,
                  .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                  .pointCount  = 30u,
              });

    drawShape(offset,
              "Star",
              sf::StarShapeData{
                  .position    = {32.f, 364.f},
                  .origin      = {64.f, 64.f},
                  .outerRadius = 64.f,
                  .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                  .pointCount  = 3u + static_cast<unsigned int>(10.f * getPhasedValue(0.1f, 2.f)),
              });

    drawShape(offset,
              "CurvedArrow",
              sf::CurvedArrowShapeData{
                  .position    = {196.f, 364.f},
                  .origin      = {64.f, 64.f},
                  .outerRadius = 64.f,
                  .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                  .startAngle  = sf::degrees(0.f),
                  .sweepAngle  = sf::degrees((270.f * getPhasedValue(0.1f, 2.f))),
                  .headLength  = 32.f,
                  .headWidth   = 8.f + (64.f * getPhasedValue(0.06f, 3.f)),
              });
}


////////////////////////////////////////////////////////////
ExampleShapes::ExampleShapes(const GameDependencies& deps) : ShowcaseExample{"Shapes"}, m_deps{deps}
{
}


////////////////////////////////////////////////////////////
void ExampleShapes::update(const float deltaTimeMs)
{
    m_time += deltaTimeMs;
}


////////////////////////////////////////////////////////////
void ExampleShapes::draw()
{
    m_phase = 0.f;

    for (const auto offset : {sf::Vec2f{0.f, 0.f}, {492.f, 0.f}, {0.f, 492.f}, {492.f, 492.f}})
        drawAllShapes(offset);
}
