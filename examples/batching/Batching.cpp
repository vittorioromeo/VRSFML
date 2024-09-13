#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"

#include <imgui.h>

#include <random>
#include <string>
#include <vector>

#include <cstddef>


float getRndFloat(float min, float max)
{
    static std::mt19937 rng(std::random_device{}());
    return std::uniform_real_distribution<float>{min, max}(rng);
}

int main()
{
    //
    //
    // Set up graphics context
    sf::GraphicsContext graphicsContext;

    //
    //
    // Set up window
    constexpr sf::Vector2f windowSize{1024.f, 768.f};

    sf::RenderWindow window(graphicsContext,
                            {.size{windowSize.toVector2u()},
                             .title     = "Vittorio's SFML fork: batching example",
                             .resizable = false});

    window.setVerticalSyncEnabled(true);

    //
    //
    // Set up imgui
    sf::ImGui::ImGuiContext imGuiContext(graphicsContext);
    if (!imGuiContext.init(window))
        return -1;

    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create(graphicsContext, {1024u, 1024u}).value()};
    textureAtlas.getTexture().setSmooth(true);

    //
    //
    // Load fonts
    const auto fontTuffy = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf", &textureAtlas).value();
    const auto fontMouldyCheese = sf::Font::openFromFile(graphicsContext, "resources/mouldycheese.ttf", &textureAtlas).value();

    //
    //
    // Load images
    const auto imgElephant = sf::Image::loadFromFile("resources/elephant.png").value();
    const auto imgGiraffe  = sf::Image::loadFromFile("resources/giraffe.png").value();
    const auto imgMonkey   = sf::Image::loadFromFile("resources/monkey.png").value();
    const auto imgPig      = sf::Image::loadFromFile("resources/pig.png").value();
    const auto imgRabbit   = sf::Image::loadFromFile("resources/rabbit.png").value();
    const auto imgSnake    = sf::Image::loadFromFile("resources/snake.png").value();

    //
    //
    // Add images to texture atlas
    const sf::FloatRect spriteTextureRects[]{{textureAtlas.add(imgElephant).value(), imgElephant.getSize().toVector2f()},
                                             {textureAtlas.add(imgGiraffe).value(), imgGiraffe.getSize().toVector2f()},
                                             {textureAtlas.add(imgMonkey).value(), imgMonkey.getSize().toVector2f()},
                                             {textureAtlas.add(imgPig).value(), imgPig.getSize().toVector2f()},
                                             {textureAtlas.add(imgRabbit).value(), imgRabbit.getSize().toVector2f()},
                                             {textureAtlas.add(imgSnake).value(), imgSnake.getSize().toVector2f()}};

    //
    //
    // Simulation stuff
    constexpr const char* names[]{"Elephant", "Giraffe", "Monkey", "Pig", "Rabbit", "Snake"};

    struct Entity
    {
        sf::Text     text;
        sf::Sprite   sprite;
        sf::Vector2f velocity;
        float        torque;
    };

    std::vector<Entity> entities;

    const auto populateEntities = [&](const std::size_t n)
    {
        if (n < entities.size())
        {
            entities.erase(entities.begin() + static_cast<std::ptrdiff_t>(n), entities.end());
            return;
        }

        entities.clear();
        entities.reserve(n);

        for (std::size_t i = 0u; i < n; ++i)
        {
            const std::size_t    type        = i % 6u;
            const sf::FloatRect& textureRect = spriteTextureRects[type];

            const auto label = std::string{names[i % 6u]} + " #" + std::to_string((i / (type + 1)) + 1);

            auto& [text,
                   sprite,
                   velocity,
                   torque] = entities.emplace_back(sf::Text{i % 2u == 0u ? fontTuffy : fontMouldyCheese, label},
                                                   sf::Sprite{textureRect},
                                                   sf::Vector2f{getRndFloat(-2.5f, 2.5f), getRndFloat(-2.5f, 2.5f)},
                                                   getRndFloat(-0.05f, 0.05f));

            sprite.setOrigin(textureRect.size / 2.f);
            sprite.setRotation(sf::degrees(getRndFloat(0.f, 360.f)));

            const float scaleFactor = getRndFloat(0.08f, 0.17f);
            sprite.setScale({scaleFactor, scaleFactor});
            text.setScale({scaleFactor * 3.5f, scaleFactor * 3.5f});

            sprite.setPosition({getRndFloat(0.f, windowSize.x), getRndFloat(0.f, windowSize.y)});

            text.setFillColor(sf::Color::Black);
            text.setOutlineColor(sf::Color::White);
            text.setOutlineThickness(5.f);

            text.setOrigin(text.getLocalBounds().size / 2.f);
        }
    };

    //
    //
    // Set up UI elements
    bool         useBatch      = true;
    bool         drawSprites   = true;
    bool         drawText      = true;
    int          numEntities   = 100;
    unsigned int drawnVertices = 0u;

    //
    //
    // Set up drawable batch
    sf::DrawableBatch drawableBatch;

    //
    //
    // Set up clock and time sampling stuff
    sf::Clock clock;
    sf::Clock fpsClock;

    std::vector<float> samplesUpdateMs{};
    std::vector<float> samplesDrawMs{};
    std::vector<float> samplesFPS{};

    const auto recordUs = [&](std::vector<float>& target, const float value)
    {
        target.push_back(value);

        if (target.size() > 32u)
            target.erase(target.begin());
    };

    const auto getAverage = [&](const std::vector<float>& target)
    {
        double accumulator = 0.0;

        for (auto value : target)
            accumulator += static_cast<double>(value);

        return accumulator / static_cast<double>(target.size());
    };

    //
    //
    // Set up initial simulation state
    populateEntities(static_cast<std::size_t>(numEntities));

    //
    //
    // Simulation loop
    while (true)
    {
        fpsClock.restart();

        ////////////////////////////////////////////////////////////
        // Event handling
        ////////////////////////////////////////////////////////////
        while (sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        {
            clock.restart();

            for (auto& [text, sprite, velocity, torque] : entities)
            {
                sprite.move(velocity);
                sprite.rotate(sf::radians(torque));

                if ((sprite.getPosition().x > windowSize.x && velocity.x > 0.f) ||
                    (sprite.getPosition().x < 0.f && velocity.x < 0.f))
                    velocity.x = -velocity.x;

                if ((sprite.getPosition().y > windowSize.y && velocity.y > 0.f) ||
                    (sprite.getPosition().y < 0.f && velocity.y < 0.f))
                    velocity.y = -velocity.y;

                text.setPosition(sprite.getPosition() - sf::Vector2f{0.f, 250.f * sprite.getScale().x});
            }

            recordUs(samplesUpdateMs, clock.getElapsedTime().asSeconds() * 1000.f);
        }

        ////////////////////////////////////////////////////////////
        // ImGui step
        ////////////////////////////////////////////////////////////
        {
            imGuiContext.update(window, fpsClock.getElapsedTime());

            ImGui::Begin("Vittorio's SFML fork: batching example", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize(ImVec2{350.f, 304.f});

            const auto clearSamples = [&]
            {
                samplesUpdateMs.clear();
                samplesDrawMs.clear();
                samplesFPS.clear();
            };

            if (ImGui::Checkbox("Enable batch drawing", &useBatch))
                clearSamples();

            if (ImGui::Checkbox("Draw sprites", &drawSprites))
                clearSamples();

            ImGui::SameLine();
            if (ImGui::Checkbox("Draw texts (expensive!)", &drawText))
                clearSamples();

            ImGui::NewLine();

            ImGui::Text("Number of entities:");
            ImGui::InputInt("##InputInt", &numEntities);

            if (ImGui::Button("Repopulate") && numEntities > 0)
                populateEntities(static_cast<std::size_t>(numEntities));

            ImGui::NewLine();

            const auto plotGraph =
                [&](const char* label, const char* unit, const std::vector<float>& samples, float upperBound)
            {
                ImGui::PlotLines(label,
                                 samples.data(),
                                 static_cast<int>(samples.size()),
                                 0,
                                 (std::to_string(getAverage(samples)) + unit).c_str(),
                                 0.f,
                                 upperBound,
                                 ImVec2{256.f, 32.f});
            };

            plotGraph("Update (ms)", " ms", samplesUpdateMs, 10.f);
            plotGraph("Draw (ms)", " ms", samplesDrawMs, 100.f);
            plotGraph("FPS", " FPS", samplesFPS, 300.f);

            ImGui::Spacing();
            ImGui::Text("Drawn vertices: %u", drawnVertices);

            ImGui::End();
        }

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        {
            clock.restart();

            window.clear();
            drawableBatch.clear();

            drawnVertices = 0u;

            for (const Entity& entity : entities)
            {
                if (drawSprites)
                {
                    if (useBatch)
                        drawableBatch.add(entity.sprite);
                    else
                        window.draw(entity.sprite, textureAtlas.getTexture());

                    drawnVertices += 4u;
                }

                if (drawText)
                {
                    if (useBatch)
                        drawableBatch.add(entity.text);
                    else
                        window.draw(entity.text);

                    drawnVertices += entity.text.getVertices().size();
                }
            }

            if (useBatch)
                window.draw(drawableBatch, {.texture = &textureAtlas.getTexture()});

            recordUs(samplesDrawMs, clock.getElapsedTime().asSeconds() * 1000.f);
        }

        imGuiContext.render(window);
        window.display();

        recordUs(samplesFPS, 1.f / fpsClock.getElapsedTime().asSeconds());
    }
}
