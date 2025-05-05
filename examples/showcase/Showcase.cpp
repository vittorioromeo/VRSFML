#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/StarShapeData.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/RectUtils.hpp"

#include "ExampleUtils.hpp"

#include <iostream>
#include <utility>
#include <vector>

#include <cmath>
#include <cstddef>


////////////////////////////////////////////////////////////
constexpr sf::Vector2f resolution{1024.f, 768.f};


////////////////////////////////////////////////////////////
class Game
{
private:
    sf::RenderWindow& m_window;
    float             m_time = 0.f;

    void exampleImmediateShapeDrawing()
    {
        float        phase = 0.f;
        sf::Vector2f offset;

        auto applyCommonSettings = [&](auto shapeData)
        {
            const auto fillColor = sf::Color::Red.withHueMod(m_time + phase * 65.f);

            shapeData.position += offset;
            shapeData.position += {64.f, 64.f};

            shapeData.fillColor        = fillColor;
            shapeData.outlineColor     = fillColor.withHueMod(120.f);
            shapeData.outlineThickness = std::abs(4.f * std::sin(m_time * 0.05f + phase));

            shapeData.rotation = sf::degrees(std::fmod(m_time * 1.f + phase * 45.f, 360.f));

            phase += 0.1f;

            return shapeData;
        };

        auto drawTheShapes = [&]
        {
            m_window.draw(applyCommonSettings(sf::CircleShapeData{
                .position   = {32.f, 32.f},
                .origin     = {64.f, 64.f},
                .radius     = 64.f,
                .pointCount = 32u,
            }));

            m_window.draw(applyCommonSettings(sf::EllipseShapeData{
                .position         = {196.f, 32.f},
                .origin           = {64.f, 32.f},
                .horizontalRadius = 64.f,
                .verticalRadius   = 32.f,
                .pointCount       = 32u,
            }));

            m_window.draw(applyCommonSettings(sf::PieSliceShapeData{
                .position   = {364.f, 32.f},
                .origin     = {64.f, 64.f},
                .radius     = 64.f,
                .startAngle = sf::degrees(0.f),
                .sweepAngle = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f)))),
                .pointCount = 32u,
            }));

            m_window.draw(applyCommonSettings(sf::RectangleShapeData{
                .position = {32.f, 196.f},
                .origin   = {64.f, 32.f},
                .size     = {128.f, 64.f},
            }));

            m_window.draw(applyCommonSettings(sf::RoundedRectangleShapeData{
                .position         = {196.f, 196.f},
                .origin           = {64.f, 32.f},
                .size             = {128.f, 64.f},
                .cornerRadius     = 32.f,
                .cornerPointCount = 16u,
            }));

            m_window.draw(applyCommonSettings(sf::RingPieSliceShapeData{
                .position    = {364.f, 196.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.2f + phase * 0.75f))),
                .startAngle  = sf::degrees(0.f),
                .sweepAngle  = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 3.5f)))),
                .pointCount  = 32u,
            }));

            m_window.draw(applyCommonSettings(sf::RingShapeData{
                .position    = {364.f, 356.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                .pointCount  = 30u,
            }));

            m_window.draw(applyCommonSettings(sf::StarShapeData{
                .position    = {32.f, 356.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                .pointCount  = 3u + static_cast<unsigned int>(10.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f))),
            }));

            m_window.draw(applyCommonSettings(sf::ArrowShapeData{
                .position    = {196.f, 356.f},
                .origin      = {(64.f + 48.f) / 2.f, 0.f},
                .shaftLength = 64.f,
                .shaftWidth  = 32.f + (32.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f))),
                .headLength  = 48.f,
                .headWidth   = 96.f - (64.f * std::abs(std::sin(m_time * 0.1f + phase * 3.f))),
            }));
        };

        drawTheShapes();
        offset.x += 500.f;
        drawTheShapes();
        offset.x -= 500.f;
        offset.y += 500.f;
        drawTheShapes();
        offset.x += 500.f;
        drawTheShapes();
    }

public:
    explicit Game(sf::RenderWindow& window) : m_window{window}
    {
    }

    [[nodiscard]] bool run()
    {
        while (true)
        {
            while (sf::base::Optional event = m_window.pollEvent())
            {
                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return true;

                if (handleAspectRatioAwareResize(*event, resolution, m_window))
                    continue;
            }

            m_time += 0.1f;

            m_window.clear();

            exampleImmediateShapeDrawing();

            const auto result = m_window.display();
            std::cout << result.drawCalls << " draw calls\r";
        }

        return true;
    }
};

int main()
{
    //
    //
    // Set up audio and graphics contexts
    auto audioContext    = sf::AudioContext::create().value();
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    auto window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVector2u(),
         .title           = "Showcase",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 8u}});

    //
    //
    // Set up game and simulation loop
    Game game{window};

    if (!game.run())
        return 1;

    return 0;
}
