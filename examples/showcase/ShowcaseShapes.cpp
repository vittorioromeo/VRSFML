#include "ShowcaseExample.hpp"
#include "ShowcaseShapes.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/ChevronShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/CogShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/CrossShapeData.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/HeartShapeData.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TrapezoidShapeData.hpp"
#include "SFML/Graphics/BatchedGeometry.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Math/Sin.hpp"


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
sf::BatchedGeometry ExampleShapes::drawShape(const char* label, const auto& shapeData)
{
    const auto offset = sf::Vec2f{32.f, 32.f};

    auto result = m_deps.rtGame->draw(applyCommonSettings(offset, shapeData), {.view = *m_deps.view});

    m_deps.rtGame->draw(*m_deps.font,
                        sf::TextData{
                            .position         = shapeData.position + offset,
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
void ExampleShapes::drawShapeAtCell(const sf::Vec2f cellPosition, const unsigned int shapeIndex)
{
    // Cycle through the 15 shape types by index. Each case constructs the shape at the given
    // cell and forwards it to `drawShape`, which handles per-frame color/rotation animation.
    switch (shapeIndex % 15u)
    {
        case 0u:
            drawShape("Circle",
                      sf::CircleShapeData{
                          .position   = cellPosition,
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .pointCount = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.04f, 2.f)),
                      });
            break;

        case 1u:
            drawShape("Ellipse",
                      sf::EllipseShapeData{
                          .position         = cellPosition,
                          .origin           = {64.f, 32.f},
                          .horizontalRadius = 64.f,
                          .verticalRadius   = 32.f,
                          .pointCount       = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.06f, 3.5f)),
                      });
            break;

        case 2u:
            drawShape("PieSlice",
                      sf::PieSliceShapeData{
                          .position   = cellPosition,
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .startAngle = sf::degrees(0.f),
                          .sweepAngle = sf::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                          .pointCount = 32u,
                      });
            break;

        case 3u:
            drawShape("Rectangle",
                      sf::RectangleShapeData{
                          .position = cellPosition,
                          .origin   = {64.f, 64.f},
                          .size = {64.f + 64.f * getPhasedValue(0.06f, 2.f), 128.f - 64.f * getPhasedValue(0.06f, 2.f)},
                      });
            break;

        case 4u:
            drawShape("RoundedRectangle",
                      sf::RoundedRectangleShapeData{
                          .position         = cellPosition,
                          .origin           = {64.f, 32.f},
                          .size             = {128.f, 64.f},
                          .cornerRadius     = 3.f + (29.f * getPhasedValue(0.1f, 1.5f)),
                          .cornerPointCount = 16u,
                      });
            break;

        case 5u:
            drawShape("Arrow",
                      sf::ArrowShapeData{
                          .position    = cellPosition,
                          .origin      = {(64.f + 48.f) / 2.f, 0.f},
                          .shaftLength = 64.f,
                          .shaftWidth  = 32.f + (32.f * getPhasedValue(0.04f, 2.f)),
                          .headLength  = 48.f,
                          .headWidth   = 96.f - (64.f * getPhasedValue(0.06f, 3.f)),
                      });
            break;

        case 6u:
            drawShape("CurvedArrow",
                      sf::CurvedArrowShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .startAngle  = sf::degrees(0.f),
                          .sweepAngle  = sf::degrees((270.f * getPhasedValue(0.1f, 2.f))),
                          .headLength  = 32.f,
                          .headWidth   = 8.f + (64.f * getPhasedValue(0.06f, 3.f)),
                      });
            break;

        case 7u:
            drawShape("Ring",
                      sf::RingShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .pointCount  = 30u,
                      });
            break;

        case 8u:
            drawShape("RingPieSlice",
                      sf::RingPieSliceShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.2f, 0.75f)),
                          .startAngle  = sf::degrees(0.f),
                          .sweepAngle  = sf::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                          .pointCount  = 32u,
                      });
            break;

        case 9u:
            drawShape("Star",
                      sf::StarShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .pointCount  = 3u + static_cast<unsigned int>(10.f * getPhasedValue(0.1f, 2.f)),
                      });
            break;

        case 10u:
            drawShape("Cross",
                      sf::CrossShapeData{
                          .position     = cellPosition,
                          .origin       = {64.f, 64.f},
                          .size         = {128.f, 128.f},
                          .armThickness = 16.f + (32.f * getPhasedValue(0.1f, 2.f)),
                      });
            break;

        case 11u:
        {
            sf::TrapezoidShapeData sd{
                .position    = cellPosition,
                .topWidth    = 32.f + (64.f * getPhasedValue(0.08f, 2.f)),
                .bottomWidth = 128.f,
                .height      = 128.f,
            };
            sd.origin = sd.getCentroid();
            drawShape("Trapezoid", sd);
            break;
        }

        case 12u:
        {
            sf::ChevronShapeData sd{
                .position  = cellPosition,
                .size      = {128.f, 128.f},
                .thickness = 16.f + (24.f * getPhasedValue(0.1f, 2.f)),
            };
            sd.origin = sd.getCentroid();
            drawShape("Chevron", sd);
            break;
        }

        case 13u:
        {
            sf::HeartShapeData sd{
                .position   = cellPosition,
                .size       = {128.f, 128.f},
                .pointCount = 32u + static_cast<unsigned int>(32.f * getPhasedValue(0.05f, 2.f)),
            };
            sd.origin = sd.getCentroid();
            drawShape("Heart", sd);
            break;
        }

        case 14u:
            drawShape("Cog",
                      sf::CogShapeData{
                          .position        = cellPosition,
                          .origin          = {64.f, 64.f},
                          .outerRadius     = 64.f,
                          .innerRadius     = 32.f + (16.f * getPhasedValue(0.15f, 2.f)),
                          .toothCount      = 6u + static_cast<unsigned int>(10.f * getPhasedValue(0.06f, 2.f)),
                          .toothWidthRatio = 0.35f + (0.3f * getPhasedValue(0.12f, 2.f)),
                      });
            break;
    }
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

    // 6 x 6 grid sized to fill the 1016 x 1016 render target exactly: 6 cells of 164 px span
    // 984 px, leaving a 16 px border on each side. Shape types cycle (36 cells / 15 shapes),
    // so the first 6 shape types appear three times and the remaining 9 appear twice.
    constexpr unsigned int gridSize = 6u;
    constexpr float        cellStep = 164.f;

    for (unsigned int row = 0u; row < gridSize; ++row)
        for (unsigned int col = 0u; col < gridSize; ++col)
        {
            const sf::Vec2f cellPosition{static_cast<float>(col) * cellStep, static_cast<float>(row) * cellStep};
            drawShapeAtCell(cellPosition, row * gridSize + col);
        }
}
