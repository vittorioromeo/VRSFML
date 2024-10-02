
#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/LayoutUtils.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"

#include <imgui.h>

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <cstddef>


////////////////////////////////////////////////////////////

#if 1

int main()
{
    sf::GraphicsContext graphicsContext;
    sf::RenderWindow    window(graphicsContext, {.size{800u, 600u}, .title = L"महसुस"});

    sf::RectangleShape rs0(
        {.position         = {250.f, 250.f},
         .origin           = {0.f, 0.f},
         .fillColor        = sf::Color::Red,
         .outlineColor     = sf::Color::Yellow,
         .outlineThickness = 3.f,
         .size             = {64.f, 64.f}});

    sf::RectangleShape cs0(
        {.position         = {450.f, 450.f},
         .origin           = {-25.f, 50.f},
         .fillColor        = sf::Color::Blue,
         .outlineColor     = sf::Color::Yellow,
         .outlineThickness = 2.f,
         .size             = {36.f, 36.f}});

    sf::setTopRight(rs0, sf::getTopRight(window));

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        window.draw(rs0, /* texture */ nullptr);
        window.draw(cs0, /* texture */ nullptr);

        cs0.rotation += sf::radians(0.0005f);
        sf::setCenter(cs0, sf::getBottomLeft(rs0));

        window.display();
    }
}

#elif 0

int main()
{
    sf::GraphicsContext graphicsContext;

    sf::RenderWindow window(graphicsContext, {.size{800u, 600u}, .title = L"महसुस"});

    auto textureAtlas = sf::TextureAtlas{sf::Texture::create(graphicsContext, {1024u, 1024u}).value()};

    const auto font0 = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf", &textureAtlas).value();
    const auto font1 = sf::Font::openFromFile(graphicsContext, "resources/mouldycheese.ttf", &textureAtlas).value();

    const auto sfmlLogoImage     = sf::Image::loadFromFile("resources/sfml_logo.png").value();
    const auto sfmlLogoAtlasRect = textureAtlas.add(sfmlLogoImage).value();

    const auto whiteDotAtlasRect = textureAtlas.add(graphicsContext.getBuiltInWhiteDotTexture()).value();

    sf::Sprite sfmlLogo(sfmlLogoAtlasRect);

    const sf::Text text0(font0, {.position = {0u, 0u}, .string = "Test", .characterSize = 128u});
    const sf::Text text1(font0, {.position = {128u, 0u}, .string = "acbasdfbFOOBAR", .characterSize = 32u});
    const sf::Text text2(font0, {.position = {0u, 128u}, .string = "ssdfbsdbfudsy", .characterSize = 64u});
    const sf::Text text3(font1, {.position = {128u, 128u}, .string = "Test", .characterSize = 128u});
    const sf::Text text4(font1, {.position = {256u, 128u}, .string = "FOmfgj,ryfkmtdfOBAR", .characterSize = 32u});
    const sf::Text text5(font1, {.position = {128u, 256u}, .string = "abscas", .characterSize = 64u});

    const sf::CircleShape circle0{
        {.position           = {350.f, 350.f},
         .scale              = {2.f, 2.f},
         .textureRect        = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
         .outlineTextureRect = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
         .fillColor          = sf::Color::Red,
         .outlineColor       = sf::Color::Yellow,
         .outlineThickness   = 8.f,
         .radius             = 45.f}};

    // Create drawable batch to optimize rendering
    sf::CPUDrawableBatch drawableBatch;

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        {
            drawableBatch.clear();

            drawableBatch.add(text0);

            sfmlLogo.position = {170.f, 50.f};
            sfmlLogo.scale    = {1.5f, 1.5f};
            drawableBatch.add(sfmlLogo);

            drawableBatch.add(text1);
            sfmlLogo.position = {100.f, 50.f};
            sfmlLogo.scale    = {1.0f, 1.0f};
            drawableBatch.add(sfmlLogo);

            drawableBatch.add(text2);
            sfmlLogo.position = {300.f, 150.f};
            sfmlLogo.scale    = {1.5f, 1.5f};
            drawableBatch.add(sfmlLogo);

            drawableBatch.add(text3);
            sfmlLogo.position = {250.f, 250.f};
            sfmlLogo.scale    = {1.0f, 1.0f};
            drawableBatch.add(sfmlLogo);

            drawableBatch.add(text4);
            drawableBatch.add(text5);

            drawableBatch.add(circle0);

            window.draw(drawableBatch, {.texture = &textureAtlas.getTexture()});
        }

        // window.draw(circle0, /* texture */ nullptr);

        window.display();
    }

    return -100;
}

#elif defined(FOOOO)

int main()
{
    const float screenWidth  = 800.0f;
    const float screenHeight = 600.0f;

    const sf::Vector2u screenSize{static_cast<unsigned int>(screenWidth), static_cast<unsigned int>(screenHeight)};

    sf::GraphicsContext graphicsContext;

    std::cout << sf::Texture::getMaximumSize(graphicsContext) << '\n';
    return 0;

    // TODO P0: aa level of 4 causes glcheck assert fail on opengl

    sf::RenderWindow window(graphicsContext,
                            {.size{screenSize},
                             .title = "Window",
                             .vsync = true,
                             .contextSettings{.depthBits = 0, .stencilBits = 0, .antiAliasingLevel = 4}});

    auto image   = sf::Image::create(screenSize, sf::Color::White).value();
    auto texture = sf::Texture::loadFromImage(graphicsContext, image).value();

    auto baseRenderTexture = sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/})
                                 .value();

    sf::RenderTexture renderTextures
        [2]{sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value(),
            sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value()};

    std::vector<sf::Vertex> vertexArrays[2];

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        vertexArrays[0].clear();
        vertexArrays[1].clear();

        float xCenter = screenWidth / 2;

        vertexArrays[0].emplace_back(sf::Vector2f{0, 0}, sf::Color::White, sf::Vector2f{0, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});

        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        // right half of screen
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, screenHeight},
                                     sf::Color::White,
                                     sf::Vector2f{screenWidth, screenHeight});

        renderTextures[0].clear();
        renderTextures[1].clear();

        sf::Sprite sprite(texture.getRect());
        renderTextures[0].draw(sprite, texture);

        sprite.color = sf::Color::Green;
        renderTextures[1].draw(sprite, texture);

        baseRenderTexture.clear();


        renderTextures[0].display();
        baseRenderTexture.draw(vertexArrays[0],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[0].getTexture()});

        renderTextures[1].display();
        baseRenderTexture.draw(vertexArrays[1],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[1].getTexture()});

        baseRenderTexture.display();

        window.draw(sf::Sprite(baseRenderTexture.getTexture().getRect()), baseRenderTexture.getTexture());

        window.display();
    }

    return 0;
}

#elif defined(BARABARAR)

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/String.hpp"

#include <sstream>

int main()
{
    sf::GraphicsContext graphicsContext;

    sf::RenderWindow window(graphicsContext, {.size{800u, 600u}, .title = "Test", .vsync = false, .resizable = false});

    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    sf::Text text(font, "Test", 20);

    sf::RenderTexture renderTexture[10]{sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value()};

    ;

    sf::Clock          clock;
    std::ostringstream sstr;

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        for (int i = 0; i < 20; ++i)
        {
            for (auto& j : renderTexture)
            {
                j.clear(sf::Color(0, 0, 0));

                for (auto& u : renderTexture)
                    u.draw(text);

                j.display();
            }

            window.clear(sf::Color(0, 0, 0));

            for (const auto& j : renderTexture)
            {
                sf::Sprite sprite(j.getTexture().getRect());

                for (int k = 0; k < 10; ++k)
                    window.draw(sprite, j.getTexture());
            }
        }

        window.display();

        sstr.str("");
        sstr << "Test -- Frame: " << clock.restart().asSeconds() << " sec";

        window.setTitle(sstr.str());
    }

    return EXIT_SUCCESS;
}

#else

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/String.hpp"

#include <cstdlib>

int main()
{
    sf::GraphicsContext graphicsContext;

    const auto       font         = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();
    const sf::String textContents = "abcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\n";

    auto text          = sf::Text(font, textContents);
    auto renderTexture = sf::RenderTexture::create(graphicsContext, {1680, 1050}).value();

    renderTexture.clear();

    for (std::size_t i = 0; i < 100'000; ++i)
    {
        text.setOutlineThickness(static_cast<float>(5 + (i % 2)));
        renderTexture.draw(text);
    }

    renderTexture.display();
}

#endif
