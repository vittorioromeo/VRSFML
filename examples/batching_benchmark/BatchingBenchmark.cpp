#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
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

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Optional.hpp"

#include <iostream>
#include <random>
#include <vector>

#include <cstddef>
#include <cstdio>

int main()
{
    //
    //
    // Set up random generator
    // std::minstd_rand rng(std::random_device{}());
    std::minstd_rand rng(100);
    const auto getRndFloat = [&](float min, float max) { return std::uniform_real_distribution<float>{min, max}(rng); };

    //
    //
    // Set up graphics context
    sf::GraphicsContext graphicsContext;

    //
    //
    // Set up window
    constexpr sf::Vector2f windowSize{1680.f, 1050.f};

    sf::RenderWindow window(graphicsContext,
                            {.size{windowSize.toVector2u()},
                             .title     = "Vittorio's SFML fork: batching benchmark",
                             .resizable = false,
                             .vsync     = true});

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
    struct Entity
    {
        sf::Text   text;
        sf::Sprite sprite;
        float      torque;
    };

    std::vector<Entity> entities;

    const auto populateEntities = [&](const std::size_t n)
    {
        entities.clear();
        entities.reserve(n);

        for (std::size_t i = 0u; i < n; ++i)
        {
            const std::size_t    type        = i % 6u;
            const sf::FloatRect& textureRect = spriteTextureRects[type];

            auto& [text, sprite, torque] = entities.emplace_back(sf::Text{i % 2u == 0u ? fontTuffy : fontMouldyCheese,
                                                                          {.string           = "abcdefABCDEF",
                                                                           .fillColor        = sf::Color::Black,
                                                                           .outlineColor     = sf::Color::White,
                                                                           .outlineThickness = 5.f}},
                                                                 sf::Sprite{textureRect},
                                                                 getRndFloat(-0.05f, 0.05f));

            sprite.origin   = textureRect.size / 2.f;
            sprite.rotation = sf::radians(getRndFloat(0.f, sf::base::tau));

            const float scaleFactor = getRndFloat(0.08f, 0.17f);
            sprite.scale            = {scaleFactor, scaleFactor};
            text.scale              = sprite.scale * 3.5f;

            sprite.position = {getRndFloat(0.f, windowSize.x), getRndFloat(0.f, windowSize.y)};

            text.origin   = text.getLocalBounds().size / 2.f;
            text.position = sprite.position;
        }
    };

    //
    //
    // Set up UI elements
    bool useBatch    = true;
    bool drawSprites = false;
    bool drawText    = true;
    int  numEntities = 50'000;
    int  numFrames   = 240;

    //
    //
    // Set up benchmark
    sf::DrawableBatch drawableBatch;
    populateEntities(static_cast<std::size_t>(numEntities));

    sf::Clock  clock;
    const auto startTime = clock.getElapsedTime();

    while (--numFrames > 0)
    {
        window.clear();
        drawableBatch.clear();

        while (window.pollEvent())
            ;

        for (const Entity& entity : entities)
        {
            if (drawSprites)
            {
                if (useBatch)
                    drawableBatch.add(entity.sprite);
                else
                    window.draw(entity.sprite, textureAtlas.getTexture());
            }

            if (drawText)
            {
                if (useBatch)
                    drawableBatch.add(entity.text);
                else
                    window.draw(entity.text);
            }
        }

        if (useBatch)
            window.draw(drawableBatch, {.texture = &textureAtlas.getTexture()});

        window.display();
    }

    const auto finalTime = clock.getElapsedTime() - startTime;

    std::cout << "FINAL TIME: " << finalTime.asMilliseconds() << " ms\n";
}
