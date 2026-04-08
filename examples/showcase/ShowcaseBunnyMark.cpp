#include "ShowcaseBunnyMark.hpp"
#include "ShowcaseExample.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"


////////////////////////////////////////////////////////////
sf::Rect2f ExampleBunnyMark::addImgToAtlasWithRotatedHue(const sf::Path& path, const float hueDegrees)
{
    auto img = sf::Image::loadFromFile(path).value();
    img.rotateHue(hueDegrees);
    return m_textureAtlas.add(img).value();
}


////////////////////////////////////////////////////////////
sf::base::String ExampleBunnyMark::toDigitSeparatedString(const sf::base::SizeT value)
{
    auto s = sf::base::toString(value);

    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
        s.insert(static_cast<sf::base::SizeT>(i), ".");

    return s;
}


////////////////////////////////////////////////////////////
ExampleBunnyMark::ExampleBunnyMark(const GameDependencies& deps, sf::TextureAtlas& textureAtlas) :
    ShowcaseExample{"Bunnymark"},
    m_deps{deps},
    m_textureAtlas{textureAtlas},
    m_bunnyTextureRects{
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 0.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 45.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 90.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 135.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 180.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 225.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 270.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 315.f),
    }
{
}


////////////////////////////////////////////////////////////
void ExampleBunnyMark::update(const float deltaTimeMs)
{
    m_time += deltaTimeMs;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        m_bunnyTargetCount += 1000;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        m_bunnyTargetCount -= 1000;

    m_bunnyTargetCount = sf::base::clamp(m_bunnyTargetCount, sf::base::SizeT{1000}, sf::base::SizeT{2'500'000});

    if (m_bunnies.size() < m_bunnyTargetCount)
    {
        m_bunnies.reserve(m_bunnyTargetCount);

        for (sf::base::SizeT i = m_bunnies.size(); i < m_bunnyTargetCount; ++i)
        {
            m_bunnies.emplaceBack(
                /* position */ m_rng.getVec2f(resolution),
                /* velocity */ m_rng.getVec2f({-1.f, -1.f}, {1.f, 1.f}),
                /* rotation */ sf::radians(m_rng.getF(0.f, sf::base::tau)),
                /*    scale */ m_rng.getF(0.25f, 0.5f));
        }
    }
    else if (m_bunnies.size() > m_bunnyTargetCount)
    {
        m_bunnies.resize(m_bunnyTargetCount);
    }

    for (auto& [position, velocity, rotation, scale] : m_bunnies)
    {
        position += velocity * deltaTimeMs;

        if (position.x < 0.f)
            position.x = resolution.x;
        else if (position.x > resolution.x)
            position.x = 0.f;

        if (position.y < 0.f)
            position.y = resolution.y;
        else if (position.y > resolution.y)
            position.y = 0.f;

        rotation += sf::radians(0.05f * deltaTimeMs);
    }
}


////////////////////////////////////////////////////////////
void ExampleBunnyMark::draw()
{
    sf::base::SizeT i = 0;

    {
        auto drawCtx = m_deps.rtGame->withLockedRenderStates(
            {.view = *m_deps.view, .texture = &m_textureAtlas.getTexture()});

        for (auto& [position, velocity, rotation, scale] : m_bunnies)
        {
            const auto& txr = m_bunnyTextureRects[i % 8u];

            drawCtx.draw(sf::Sprite{
                .position    = position,
                .scale       = {scale, scale},
                .origin      = txr.size / 2.f,
                .rotation    = rotation,
                .textureRect = txr,
            });

            ++i;
        }
    }

    const auto digitSeparatedBunnyCount = toDigitSeparatedString(m_bunnies.size());

    const auto vertices = m_deps.rtGame->draw(*m_deps.font,
                                              sf::TextData{
                                                  .position         = {8.f, 8.f},
                                                  .string           = digitSeparatedBunnyCount + " bunnies",
                                                  .characterSize    = 32,
                                                  .outlineColor     = sf::Color::Black,
                                                  .outlineThickness = 4.f,
                                              },
                                              {.view = *m_deps.view}); // TODO P1: add a way to prevent flushing

    for (sf::base::SizeT j = 0u; j < vertices.size(); j += 4u)
    {
        const sf::base::SizeT outlineIndependentIndex = j % (vertices.size() / 2u);

        if (outlineIndependentIndex >= digitSeparatedBunnyCount.size() * 4u)
        {
            const float offY = sf::base::sin(m_time) * 1.25f;

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
            const float offY = sf::base::sin(m_time + static_cast<float>(outlineIndependentIndex)) * 1.5f;

            vertices[j + 0].position.y += offY;
            vertices[j + 1].position.y += offY;
            vertices[j + 2].position.y += offY;
            vertices[j + 3].position.y += offY;
        }
    }

    m_deps.rtGame->draw(*m_deps.font,
                        sf::TextData{
                            .position         = {8.f, 48.f},
                            .string           = "Change number of bunnies with arrow keys",
                            .characterSize    = 16,
                            .outlineColor     = sf::Color::Black,
                            .outlineThickness = 2.f,
                        },
                        {.view = *m_deps.view});
}
