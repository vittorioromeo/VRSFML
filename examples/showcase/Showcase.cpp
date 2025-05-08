#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Vec2.hpp"

#include "ExampleUtils.hpp"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <vector>

#include <cmath>
#include <cstddef>


////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1016.f, 1016.f};


////////////////////////////////////////////////////////////
class Game
{
private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow& m_window;
    float             m_time = 0.f;

    ////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({2048u, 2048u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/tuffy.ttf", &m_textureAtlas).value();

    ////////////////////////////////////////////////////////////
    const sf::FloatRect m_bunnyTexureRects[2] = {
        addImgToAtlas("resources/bunny0.png"),
        addImgToAtlas("resources/bunny1.png"),
    };

    ////////////////////////////////////////////////////////////
    struct Bunny
    {
        sf::Vec2f position;
        sf::Vec2f velocity;
        sf::Angle rotation;
        float     scale{};
    };

    std::vector<Bunny> m_bunnies;

    ////////////////////////////////////////////////////////////
    RNGFast m_rng{/* seed */ 1234};

    ////////////////////////////////////////////////////////////
    std::size_t m_activeExample = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile(path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    void exampleImmediateShapeDrawing()
    {
        float phase = 0.f;

        auto applyCommonSettings = [&](const sf::Vec2f currentOffset, auto shapeData)
        {
            const auto fillColor = sf::Color::Red.withRotatedHue(m_time + phase * 65.f);

            shapeData.position += currentOffset;
            shapeData.position += {64.f, 64.f};

            shapeData.fillColor        = fillColor;
            shapeData.outlineColor     = fillColor.withRotatedHue(180.f);
            shapeData.outlineThickness = std::abs(4.f * std::sin(m_time * 0.05f + phase));

            shapeData.rotation = sf::degrees(std::fmod(m_time * 1.f + phase * 45.f, 360.f));

            phase += 0.1f;

            return shapeData;
        };

        auto drawShape = [&](const sf::Vec2f currentOffset, const char* label, const auto& shapeData)
        {
            m_window.draw(applyCommonSettings(currentOffset, shapeData));

            m_window.draw(m_font,
                          sf::TextData{
                              .position         = shapeData.position + currentOffset,
                              .string           = label,
                              .characterSize    = 16,
                              .outlineColor     = sf::Color::Black,
                              .outlineThickness = 2.f,
                          });
        };

        for (const auto offset : {sf::Vec2f{0.f, 0.f}, {492.f, 0.f}, {0.f, 492.f}, {492.f, 492.f}})
        {
            drawShape(offset,
                      "Circle",
                      sf::CircleShapeData{
                          .position   = {32.f, 32.f},
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .pointCount = 3u +
                                        static_cast<unsigned int>(29.f * std::abs(std::sin(m_time * 0.04f + phase * 2.f))),
                      });

            drawShape(offset,
                      "Ellipse",
                      sf::EllipseShapeData{
                          .position         = {196.f, 32.f},
                          .origin           = {64.f, 32.f},
                          .horizontalRadius = 64.f,
                          .verticalRadius   = 32.f,
                          .pointCount       = 3u + static_cast<unsigned int>(
                                                 29.f * std::abs(std::sin(m_time * 0.06f + phase * 3.5f))),
                      });

            drawShape(offset,
                      "PieSlice",
                      sf::PieSliceShapeData{
                          .position   = {364.f, 32.f},
                          .origin     = {64.f, 64.f},
                          .radius     = 64.f,
                          .startAngle = sf::degrees(0.f),
                          .sweepAngle = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f)))),
                          .pointCount = 32u,
                      });

            drawShape(offset,
                      "Rectangle",
                      sf::RectangleShapeData{
                          .position = {32.f, 196.f},
                          .origin   = {64.f, 32.f},
                          .size     = {128.f, 64.f},
                      });

            drawShape(offset,
                      "RoundedRectangle",
                      sf::RoundedRectangleShapeData{
                          .position         = {196.f, 196.f},
                          .origin           = {64.f, 32.f},
                          .size             = {128.f, 64.f},
                          .cornerRadius     = 3.f + (29.f * std::abs(std::sin(m_time * 0.1f + phase * 1.5f))),
                          .cornerPointCount = 16u,
                      });

            drawShape(offset,
                      "RingPieSlice",
                      sf::RingPieSliceShapeData{
                          .position    = {364.f, 196.f},
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.2f + phase * 0.75f))),
                          .startAngle  = sf::degrees(0.f),
                          .sweepAngle  = sf::degrees((360.f * std::abs(std::sin(m_time * 0.1f + phase * 2.0f)))),
                          .pointCount  = 32u,
                      });

            drawShape(offset,
                      "Ring",
                      sf::RingShapeData{
                          .position    = {364.f, 364.f},
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                          .pointCount  = 30u,
                      });

            drawShape(offset,
                      "Star",
                      sf::StarShapeData{
                          .position    = {32.f, 364.f},
                          .origin      = {64.f, 64.f},
                          .outerRadius = 64.f,
                          .innerRadius = 32.f + (16.f * std::abs(std::sin(m_time * 0.25f + phase * 2.f))),
                          .pointCount = 3u + static_cast<unsigned int>(10.f * std::abs(std::sin(m_time * 0.1f + phase * 2.f))),
                      });

            drawShape(offset,
                      "Arrow",
                      sf::ArrowShapeData{
                          .position    = {196.f, 364.f},
                          .origin      = {(64.f + 48.f) / 2.f, 0.f},
                          .shaftLength = 64.f,
                          .shaftWidth  = 32.f + (32.f * std::abs(std::sin(m_time * 0.04f + phase * 2.f))),
                          .headLength  = 48.f,
                          .headWidth   = 96.f - (64.f * std::abs(std::sin(m_time * 0.06f + phase * 3.f))),
                      });
        }
    }

    ////////////////////////////////////////////////////////////
    void exampleBunnymark()
    {
        static std::size_t bunnyTargetCount = 100'000;

        if (m_bunnies.size() < bunnyTargetCount)
        {
            m_bunnies.reserve(bunnyTargetCount);

            for (std::size_t i = m_bunnies.size(); i < bunnyTargetCount; ++i)
            {
                m_bunnies.emplace_back(
                    /* position */ m_rng.getVec2f(resolution),
                    /* velocity */ m_rng.getVec2f({-1.f, -1.f}, {1.f, 1.f}),
                    /* rotation */ sf::radians(m_rng.getF(0.f, sf::base::tau)),
                    /*    scale */ m_rng.getF(0.1f, 0.9f));
            }
        }
        else if (m_bunnies.size() > bunnyTargetCount)
        {
            m_bunnies.resize(bunnyTargetCount);
        }

        std::size_t i = 0u;

        for (auto& [position, velocity, rotation, scale] : m_bunnies)
        {
            const auto& txr = m_bunnyTexureRects[i % 2u];

            m_window.draw(
                sf::Sprite{
                    .position    = position,
                    .scale       = {scale, scale},
                    .origin      = txr.size / 2.f,
                    .rotation    = rotation,
                    .textureRect = txr,
                },
                {.texture = &m_textureAtlas.getTexture()});

            ++i;

            position += velocity;

            if (position.x < 0.f)
                position.x = resolution.x;
            else if (position.x > resolution.x)
                position.x = 0.f;

            if (position.y < 0.f)
                position.y = resolution.y;
            else if (position.y > resolution.y)
                position.y = 0.f;

            rotation += sf::radians(0.05f);
        }

        const auto toDigitSeparatedString = [](const std::size_t value) -> std::string
        {
            std::string s = std::to_string(value);

            for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
                s.insert(static_cast<std::size_t>(i), ".");

            return s;
        };


        const auto digitSeparatedBunnyCount = toDigitSeparatedString(m_bunnies.size());

        const auto vertices = m_window.draw(m_font,
                                            sf::TextData{
                                                .position         = {8.f, 8.f},
                                                .string           = digitSeparatedBunnyCount + " bunnies",
                                                .characterSize    = 32,
                                                .outlineColor     = sf::Color::Black,
                                                .outlineThickness = 4.f,
                                            });

        for (std::size_t j = 0u; j < vertices.size(); j += 4u)
        {
            const std::size_t outlineIndependentIndex = j % (vertices.size() / 2u);

            if (outlineIndependentIndex >= digitSeparatedBunnyCount.size() * 4u)
            {
                const float offY = std::sin(m_time) * 1.25f;

                vertices[j + 0].position.y -= offY;
                vertices[j + 1].position.y -= offY;
                vertices[j + 2].position.y += offY;
                vertices[j + 3].position.y += offY;

                vertices[j + 0].position.x -= offY;
                vertices[j + 1].position.x += offY;
                vertices[j + 2].position.x -= offY;
                vertices[j + 3].position.x += offY;
            }
            else
            {
                const float offY = std::sin(m_time + static_cast<float>(outlineIndependentIndex)) * 1.5f;

                vertices[j + 0].position.y += offY;
                vertices[j + 1].position.y += offY;
                vertices[j + 2].position.y += offY;
                vertices[j + 3].position.y += offY;
            }
        }

        m_window.draw(m_font,
                      sf::TextData{
                          .position         = {8.f, 48.f},
                          .string           = "Change number of bunnies with arrow keys",
                          .characterSize    = 16,
                          .outlineColor     = sf::Color::Black,
                          .outlineThickness = 2.f,
                      });

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            bunnyTargetCount += 1000;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            bunnyTargetCount -= 1000;

        bunnyTargetCount = std::clamp(bunnyTargetCount, std::size_t{1000}, std::size_t{2'500'000});
    }

public:
    ////////////////////////////////////////////////////////////
    explicit Game(sf::RenderWindow& window) : m_window{window}
    {
    }

    ////////////////////////////////////////////////////////////
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

                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                    if (eKeyPressed->code == sf::Keyboard::Key::Space)
                        m_activeExample = ((m_activeExample + 1u) % 2u);
            }

            m_time += 0.1f;

            m_window.clear();

            if (m_activeExample == 0u)
                exampleImmediateShapeDrawing();
            else if (m_activeExample == 1u)
                exampleBunnymark();

            const auto result = m_window.display();
            std::cout << result.drawCalls << " draw calls\r";
        }

        return true;
    }
};

////////////////////////////////////////////////////////////
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
        {.size            = resolution.toVec2u(),
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
