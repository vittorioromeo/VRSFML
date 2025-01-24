#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

namespace
{

constexpr sf::Vector2f resolution{1024.f, 768.f};
constexpr auto         resolutionUInt = resolution.toVector2u();

const sf::Vector2f boundaries{1024.f * 10.f, 768.f};

[[nodiscard]] float getRndFloat(const float min, const float max)
{
    static std::random_device         randomDevice;
    static std::default_random_engine randomEngine(randomDevice());
    return std::uniform_real_distribution<float>{min, max}(randomEngine);
}

[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f mins, const sf::Vector2f maxs)
{
    return {getRndFloat(mins.x, maxs.x), getRndFloat(mins.y, maxs.y)};
}

[[nodiscard]] sf::Vector2f getRndVector2f(const sf::Vector2f maxs)
{
    return getRndVector2f({0.f, 0.f}, maxs);
}

[[nodiscard]] float remap(float x, float oldMin, float oldMax, float newMin, float newMax)
{
    if (oldMin == oldMax)
    {
        return newMin; // or newMax, depending on your use case
    }

    // Perform the remapping
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}


struct Bubble
{
    sf::Sprite   sprite;
    sf::Vector2f velocity;
    float        radius;
};

} // namespace

int main()
{
    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    auto                    graphicsContext = sf::GraphicsContext::create().value();
    sf::ImGui::ImGuiContext imGuiContext;

    sf::RenderWindow window({.size = resolutionUInt, .title = "Bubble Idle", .vsync = true});

    if (!imGuiContext.init(window))
        return -1;

    const auto fontDailyBubble = sf::Font::openFromFile("resources/dailybubble.ttf").value();

    sf::Text moneyText{fontDailyBubble,
                       {.position         = {15.f, 70.f},
                        .string           = "$0",
                        .characterSize    = 32u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::Blue,
                        .outlineThickness = 0.5f}};

    auto txBubble128 = sf::Texture::loadFromFile("resources/bubble2.png").value();
    txBubble128.setSmooth(true);

    const auto soundBufferPop = sf::SoundBuffer::loadFromFile("resources/pop.wav").value();
    sf::Sound  soundPop(soundBufferPop);

    const float gridSize = 64.f;
    const auto  nCellsX  = static_cast<sf::base::SizeT>(sf::base::ceil(boundaries.x / gridSize)) + 1;
    const auto  nCellsY  = static_cast<sf::base::SizeT>(sf::base::ceil(boundaries.y / gridSize)) + 1;

    auto convert2DTo1D = [](sf::base::SizeT col, sf::base::SizeT row, sf::base::SizeT width)
    { return row * width + col; };

    std::vector<std::vector<sf::base::SizeT>> bubbleGrid;
    bubbleGrid.resize(nCellsX * nCellsY);

    std::vector<sf::base::SizeT> bubbleIdSet;

    std::vector<Bubble> bubbles;
    int                 money = 0;

    for (int i = 0; i < 1000; ++i)
    {
        const float scaleFactor = getRndFloat(0.05f, 0.15f) * 0.6f;

        sf::Sprite bubbleSprite{.position    = getRndVector2f(boundaries),
                                .scale       = {scaleFactor, scaleFactor},
                                .origin      = (txBubble128.getSize() / 2u).toVector2f(),
                                .textureRect = txBubble128.getRect()};

        const float radius = bubbleSprite.textureRect.size.x / 2.f * bubbleSprite.scale.x;

        bubbles.emplace_back(bubbleSprite, getRndVector2f({-0.1f, -0.1f}, {0.1f, 0.1f}), radius);
    }

    sf::Clock            fpsClock;
    sf::Clock            deltaClock;
    sf::CPUDrawableBatch cpuDrawableBatch;

    sf::base::Optional<sf::Vector2f> dragPosition;
    float                            scroll = 0.f;

    while (true)
    {
        fpsClock.restart();

        sf::base::Optional<sf::Vector2f> clickPosition;

        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* e0 = event->getIf<sf::Event::TouchMoved>())
            {
                clickPosition.emplace(e0->position.toVector2f());
            }
            else if (const auto* e1 = event->getIf<sf::Event::MouseButtonPressed>())
            {
                clickPosition.emplace(e1->position.toVector2f());

                if (!dragPosition.hasValue())
                {
                    dragPosition.emplace(e1->position.toVector2f());
                    dragPosition->x += scroll;
                }
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonReleased>())
            {
                dragPosition.reset();
            }
            else if (const auto* e1 = event->getIf<sf::Event::MouseMoved>())
            {
                if (dragPosition.hasValue())
                {
                    scroll = dragPosition->x - static_cast<float>(e1->position.x);
                }
            }
        }

        const auto deltaTime   = deltaClock.restart();
        const auto deltaTimeMs = static_cast<float>(deltaTime.asMilliseconds());

        constexpr float scrollSpeed = 2.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            dragPosition.reset();
            scroll -= scrollSpeed * deltaTimeMs;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            dragPosition.reset();
            scroll += scrollSpeed * deltaTimeMs;
        }

        scroll = sf::base::clamp(scroll, 0.f, boundaries.x - resolution.x);

        sf::View gameView{.center = {resolution.x / 2.f + scroll * 2.f, resolution.y / 2.f}, .size = resolution};
        gameView.center.x = sf::base::clamp(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f);

        for (sf::base::SizeT i = 0; i < nCellsX; ++i)
        {
            for (sf::base::SizeT j = 0; j < nCellsY; ++j)
            {
                bubbleGrid[convert2DTo1D(i, j, nCellsX)].clear();
            }
        }

        for (sf::base::SizeT i = 0; i < bubbles.size(); ++i)
        {
            Bubble& bubble = bubbles[i];

            auto& pos = bubble.sprite.position;

            const auto cellX = static_cast<sf::base::SizeT>(pos.x / gridSize);
            const auto cellY = static_cast<sf::base::SizeT>(pos.y / gridSize);

            bubbleGrid[convert2DTo1D(cellX, cellY, nCellsX)].push_back(i);
            pos += bubble.velocity * deltaTimeMs;

            if (pos.x - bubble.radius > boundaries.x)
                pos.x = -bubble.radius;
            else if (pos.x + bubble.radius < 0.f)
                pos.x = boundaries.x + bubble.radius;

            if (pos.y - bubble.radius > boundaries.y)
            {
                pos.y             = -bubble.radius;
                bubble.velocity.y = 0.f;
            }
            else if (pos.y + bubble.radius < 0.f)
                pos.y = boundaries.y + bubble.radius;

            bubble.velocity.y += 0.00005f * deltaTimeMs;

            if (clickPosition.hasValue())
            {
                window.setView(gameView);
                const auto [x, y] = window.mapPixelToCoords(clickPosition->toVector2i());

                if ((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) < bubble.radius * bubble.radius)
                {
                    std::swap(bubbles[i], bubbles.back());
                    bubbles.pop_back();
                    ++money;

                    soundPop.play(playbackDevice);
                    continue;
                }
            }
        }

        const auto handleCollision = [&](auto i, auto j)
        {
            auto&      iBubble = bubbles[i];
            auto&      jBubble = bubbles[j];
            const auto iRadius = iBubble.radius;
            const auto jRadius = jBubble.radius;

            const auto [aix, aiy] = iBubble.sprite.position;
            const auto [jx, jy]   = jBubble.sprite.position;

            if ((aix - jx) * (aix - jx) + (aiy - jy) * (aiy - jy) < iRadius * iRadius + jRadius * jRadius)
            {
                // Calculate the overlap between the bubbles
                auto  collisionVector = sf::Vector2f(jx - aix, jy - aiy); // Vector from bubble i to bubble j
                float distance        = sf::base::sqrt(collisionVector.x * collisionVector.x +
                                                collisionVector.y * collisionVector.y); // Distance between centers
                float overlap         = (iRadius + jRadius) - distance;                        // Amount of overlap

                // Normalize the collision vector
                sf::Vector2f collisionNormal = collisionVector / distance;

                // Define a "softness" factor to control how quickly the overlap is resolved
                float softnessFactor = 0.05f; // Adjust this value to control the overlap solver (0.1 = 10% per frame)

                // Calculate the displacement needed to resolve the overlap
                sf::Vector2f displacement = collisionNormal * overlap * softnessFactor;

                // Move the bubbles apart based on their masses (heavier bubbles move less)
                float m1        = iRadius * iRadius; // Mass of bubble i (quadratic scaling)
                float m2        = jRadius * jRadius; // Mass of bubble j (quadratic scaling)
                float totalMass = m1 + m2;

                iBubble.sprite.position -= displacement * (m2 / totalMass); // Move bubble i
                jBubble.sprite.position += displacement * (m1 / totalMass);
            }
        };

        for (int ix = 1; ix < static_cast<int>(nCellsX - 1); ++ix)
        {
            for (int iy = 1; iy < static_cast<int>(nCellsY - 1); ++iy)
            {
                bubbleIdSet.clear();

                for (int ox = -1; ox <= 1; ++ox)
                {
                    for (int oy = -1; oy <= 1; ++oy)
                    {
                        const auto idx = convert2DTo1D(static_cast<sf::base::SizeT>(ix + ox),
                                                       static_cast<sf::base::SizeT>(iy + oy),
                                                       nCellsX);

                        bubbleIdSet.insert(bubbleIdSet.end(), bubbleGrid[idx].begin(), bubbleGrid[idx].end());
                    }
                }

                for (sf::base::SizeT i = 0; i < bubbleIdSet.size(); ++i)
                {
                    for (sf::base::SizeT j = i + 1; j < bubbleIdSet.size(); ++j)
                    {
                        handleCollision(bubbleIdSet[i], bubbleIdSet[j]);
                    }
                }
            }
        }


        imGuiContext.update(window, deltaTime);

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear(sf::Color{84, 72, 81});


        const float progress = remap(gameView.center.x, resolution.x / 2.f, boundaries.x - resolution.x / 2.f, 0.f, 100.f);
        window.setView(gameView);

        cpuDrawableBatch.clear();

        for (auto& bubble : bubbles)
            cpuDrawableBatch.add(bubble.sprite);

        window.draw(cpuDrawableBatch, {.texture = &txBubble128});

        window.setView({.center = {resolution.x / 2.f, resolution.y / 2.f}, .size = resolution});

        moneyText.setString("$" + std::to_string(money));
        window.draw(moneyText);


        sf::RectangleShape minimapBorder{{.size = boundaries / 20.f + sf::Vector2f{6.f, 6.f}}};
        minimapBorder.position = {15.f, 15.f}; // Top-right corner
        minimapBorder.setFillColor(sf::Color::DarkBabyPink);
        minimapBorder.setOutlineColor(sf::Color::White);
        minimapBorder.setOutlineThickness(2.f);
        window.draw(minimapBorder, nullptr);

        sf::RectangleShape minimapIndicator{{.size = resolution / 20.f + sf::Vector2f{6.f, 6.f}}};
        minimapIndicator.position = {15.f + (progress / 100.f) * (boundaries.x - resolution.x) / 20.f, 15.f}; // Top-right corner
        minimapIndicator.setFillColor(sf::Color::Transparent);
        minimapIndicator.setOutlineColor(sf::Color::Red);
        minimapIndicator.setOutlineThickness(2.f);
        window.draw(minimapIndicator, nullptr);

        sf::View minimapView{.center = {(resolution.x / 2.f) * 20.f - 15.f * 20.f - 2.f * 20.f,
                                        (resolution.y / 2.f) * 20.f - 15.f * 20.f - 2.f * 20.f},
                             .size   = boundaries};
        minimapView.size = {boundaries.x / 10, boundaries.y};
        minimapView.size *= 20.f;
        window.setView(minimapView);


        window.draw(cpuDrawableBatch, {.texture = &txBubble128});

        imGuiContext.render(window);
        window.display();

        window.setTitle("FPS: " + std::to_string(1.f / fpsClock.getElapsedTime().asSeconds()));
    }
}
