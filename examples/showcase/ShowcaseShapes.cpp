#include "ShowcaseExample.hpp"
#include "ShowcaseShapes.hpp"

#include "Zancle/Graphics/ArrowShapeData.hpp"
#include "Zancle/Graphics/BatchedGeometry.hpp"
#include "Zancle/Graphics/ChevronShapeData.hpp"
#include "Zancle/Graphics/CircleShapeData.hpp"
#include "Zancle/Graphics/CogShapeData.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/CrossShapeData.hpp"
#include "Zancle/Graphics/CurvedArrowShapeData.hpp"
#include "Zancle/Graphics/EllipseShapeData.hpp"
#include "Zancle/Graphics/HeartShapeData.hpp"
#include "Zancle/Graphics/PieSliceShapeData.hpp"
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"
#include "Zancle/Graphics/RingPieSliceShapeData.hpp"
#include "Zancle/Graphics/RingShapeData.hpp"
#include "Zancle/Graphics/RoundedRectangleShapeData.hpp"
#include "Zancle/Graphics/StarShapeData.hpp"
#include "Zancle/Graphics/TextData.hpp"
#include "Zancle/Graphics/TrapezoidShapeData.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/Math/Fmod.hpp"
#include "ZancleBase/Math/Sin.hpp"


////////////////////////////////////////////////////////////
auto ExampleShapes::applyCommonSettings(const za::Vec2f currentOffset, auto shapeData)
{
    const auto fillColor = za::Color::Red.withRotatedHue(m_time + m_phase * 65.f);

    shapeData.position += currentOffset;
    shapeData.position += {64.f, 64.f};

    shapeData.fillColor        = fillColor;
    shapeData.outlineColor     = fillColor.withRotatedHue(180.f);
    shapeData.outlineThickness = zb::fabs(4.f * zb::sin(m_time * 0.05f + m_phase));

    shapeData.rotation = za::degrees(zb::fmod(m_time * 1.f + m_phase * 45.f, 360.f));

    shapeData.textureRect = {
        .position = {0.f, 0.f},
        .size     = {1.f, 1.f},
    };

    return shapeData;
}


////////////////////////////////////////////////////////////
za::BatchedGeometry ExampleShapes::drawShape(const char* label, const auto& shapeData)
{
    const auto offset = za::Vec2f{32.f, 32.f};

    auto result = m_deps.rtGame->draw(applyCommonSettings(offset, shapeData), {.view = *m_deps.view});

    m_deps.rtGame->draw(*m_deps.font,
                        za::TextData{
                            .position         = shapeData.position + offset,
                            .string           = label,
                            .characterSize    = 16,
                            .outlineColor     = za::Color::Black,
                            .outlineThickness = 2.f,
                        },
                        {.view = *m_deps.view});

    m_phase += 0.1f;

    return result;
}


////////////////////////////////////////////////////////////
float ExampleShapes::getPhasedValue(const float timeMultiplier, const float phaseMultiplier) const
{
    return zb::fabs(zb::sin(m_time * timeMultiplier + m_phase * phaseMultiplier));
}


////////////////////////////////////////////////////////////
void ExampleShapes::drawShapeAtCell(const za::Vec2f cellPosition, const unsigned int shapeIndex)
{
    // Cycle through the 15 shape types by index. Each case constructs the shape at the given
    // cell and forwards it to `drawShape`, which handles per-frame color/rotation animation.
    switch (shapeIndex % 15u)
    {
        case 0u:
            drawShape("Circle",
                      za::CircleShapeData{
                          .position   = cellPosition,
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .pointCount = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.04f, 2.f)),
                      });
            break;

        case 1u:
            drawShape("Ellipse",
                      za::EllipseShapeData{
                          .position         = cellPosition,
                          .origin           = {64.f, 32.f},
                          .horizontalRadius = 64.f,
                          .verticalRadius   = 32.f,
                          .pointCount       = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.06f, 3.5f)),
                      });
            break;

        case 2u:
            drawShape("PieSlice",
                      za::PieSliceShapeData{
                          .position   = cellPosition,
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .startAngle = za::degrees(0.f),
                          .sweepAngle = za::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                          .pointCount = 32u,
                      });
            break;

        case 3u:
        {
            const auto h = 64.f + (16.f * getPhasedValue(0.1f, 1.f));

            drawShape("Rectangle",
                      za::RectangleShapeData{
                          .position = cellPosition,
                          .origin   = {64.f, h / 2.f},
                          .size     = {128.f, h},
                      });
        }
        break;

        case 4u:
            drawShape("RoundedRectangle",
                      za::RoundedRectangleShapeData{
                          .position         = cellPosition,
                          .origin           = {64.f, 32.f},
                          .size             = {128.f, 64.f},
                          .cornerRadius     = 3.f + (29.f * getPhasedValue(0.1f, 1.5f)),
                          .cornerPointCount = 16u,
                      });
            break;

        case 5u:
            drawShape("Arrow",
                      za::ArrowShapeData{
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
                      za::CurvedArrowShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .startAngle  = za::degrees(0.f),
                          .sweepAngle  = za::degrees((270.f * getPhasedValue(0.1f, 2.f))),
                          .headLength  = 32.f,
                          .headWidth   = 8.f + (64.f * getPhasedValue(0.06f, 3.f)),
                      });
            break;

        case 7u:
            drawShape("Ring",
                      za::RingShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .pointCount  = 30u,
                      });
            break;

        case 8u:
            drawShape("RingPieSlice",
                      za::RingPieSliceShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.2f, 0.75f)),
                          .startAngle  = za::degrees(0.f),
                          .sweepAngle  = za::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                          .pointCount  = 32u,
                      });
            break;

        case 9u:
            drawShape("Star",
                      za::StarShapeData{
                          .position    = cellPosition,
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                          .pointCount  = 3u + static_cast<unsigned int>(10.f * getPhasedValue(0.1f, 2.f)),
                      });
            break;

        case 10u:
            drawShape("Cross",
                      za::CrossShapeData{
                          .position     = cellPosition,
                          .origin       = {64.f, 64.f},
                          .size         = {128.f, 128.f},
                          .armThickness = 16.f + (32.f * getPhasedValue(0.1f, 2.f)),
                      });
            break;

        case 11u:
        {
            za::TrapezoidShapeData sd{
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
            za::ChevronShapeData sd{
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
            za::HeartShapeData sd{
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
                      za::CogShapeData{
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
            const za::Vec2f cellPosition{static_cast<float>(col) * cellStep, static_cast<float>(row) * cellStep};
            drawShapeAtCell(cellPosition, row * gridSize + col);
        }
}
