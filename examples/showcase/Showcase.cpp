#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/RectUtils.hpp"

#include "ExampleUtils.hpp"

#include <iostream>
#include <utility>
#include <vector>

#include <cmath>
#include <cstddef>


////////////////////////////////////////////////////////////
constexpr sf::Vector2f resolution{1016.f, 1016.f};


////////////////////////////////////////////////////////////
class Game
{
private:
    sf::RenderWindow& m_window;
    float             m_time = 0.f;

    const sf::Font font = sf::Font::openFromFile("resources/tuffy.ttf").value();

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

        auto drawShape = [&](const auto& shapeData) { m_window.draw(applyCommonSettings(shapeData)); };

        auto drawLabel = [&](const sf::Vector2f position, const char* label)
        {
            m_window.draw(font,
                          sf::TextData{
                              .position         = position + offset,
                              .string           = label,
                              .characterSize    = 16,
                              .outlineColor     = sf::Color::Black,
                              .outlineThickness = 2.f,
                          });
        };

        auto drawTheShapes = [&]
        {
            drawShape(sf::CircleShapeData{
                .position   = {32.f, 32.f},
                .origin     = {64.f, 64.f},
                .radius     = 64.f,
                .pointCount = 3u + static_cast<unsigned int>(29.f * std::abs(std::sin(m_time * 0.04f + phase * 2.f))),
            });

            drawLabel({32.f, 32.f}, "Circle");

            drawShape(sf::EllipseShapeData{
                .position         = {196.f, 32.f},
                .origin           = {64.f, 32.f},
                .horizontalRadius = 64.f,
                .verticalRadius   = 32.f,
                .pointCount = 3u + static_cast<unsigned int>(29.f * std::abs(std::sin(m_time * 0.06f + phase * 3.5f))),
            });

            drawLabel({196.f, 32.f}, "Ellipse");

            drawShape(sf::PieSliceShapeData{
                .position   = {364.f, 32.f},
                .origin     = {64.f, 64.f},
                .radius     = 64.f,
                .startAngle = sf::degrees(0.f),
                .sweepAngle = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f)))),
                .pointCount = 32u,
            });

            drawLabel({364.f, 32.f}, "PieSlice");

            drawShape(sf::RectangleShapeData{
                .position = {32.f, 196.f},
                .origin   = {64.f, 32.f},
                .size     = {128.f, 64.f},
            });

            drawLabel({32.f, 196.f}, "Rectangle");

            drawShape(sf::RoundedRectangleShapeData{
                .position         = {196.f, 196.f},
                .origin           = {64.f, 32.f},
                .size             = {128.f, 64.f},
                .cornerRadius     = 3.f + (29.f * std::abs(std::sin(m_time * 0.1f + phase * 1.5f))),
                .cornerPointCount = 16u,
            });

            drawLabel({196.f, 196.f}, "RoundedRectangle");

            drawShape(sf::RingPieSliceShapeData{
                .position    = {364.f, 196.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.2f + phase * 0.75f))),
                .startAngle  = sf::degrees(0.f),
                .sweepAngle  = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 2.0f)))),
                .pointCount  = 32u,
            });

            drawLabel({364.f, 196.f}, "RingPieSlice");

            drawShape(sf::RingShapeData{
                .position    = {364.f, 364.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                .pointCount  = 30u,
            });

            drawLabel({364.f, 364.f}, "Ring");

            drawShape(sf::StarShapeData{
                .position    = {32.f, 364.f},
                .origin      = {64.f, 64.f},
                .outerRadius = 64.f,
                .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                .pointCount  = 3u + static_cast<unsigned int>(10.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f))),
            });

            drawLabel({32.f, 364.f}, "Star");

            drawShape(sf::ArrowShapeData{
                .position    = {196.f, 364.f},
                .origin      = {(64.f + 48.f) / 2.f, 0.f},
                .shaftLength = 64.f,
                .shaftWidth  = 32.f + (32.f * std::abs(std::sin(m_time * 0.04f + phase * 2.f))),
                .headLength  = 48.f,
                .headWidth   = 96.f - (64.f * std::abs(std::sin(m_time * 0.06f + phase * 3.f))),
            });

            drawLabel({196.f, 364.f}, "Arrow");
        };

        offset = {0.f, 0.f};
        drawTheShapes();

        offset = {492.f, 0.f};
        drawTheShapes();

        offset = {0.f, 492.f};
        drawTheShapes();

        offset = {492.f, 492.f};
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
