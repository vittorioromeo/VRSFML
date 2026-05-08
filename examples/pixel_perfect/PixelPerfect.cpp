// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Pixel-perfect scaling demo.
//
// The scene is rendered into a 320x240 render texture and presented to
// a resizable window through `computePixelPerfectView` and
// `handlePixelPerfectResize`. The viewport always covers the largest
// integer multiple of the internal resolution that fits the window;
// remaining space becomes letterbox/pillarbox bars. Resize the window
// freely to watch the scale step up and down.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/Scaling.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Event.hpp" // IWYU pragma: keep
#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{320.f, 240.f};


////////////////////////////////////////////////////////////
[[nodiscard]] sf::Image makeSmileyImage()
{
    static constexpr char pattern[16][17] = {
        "....KKKKKKKK....",
        "..KKYYYYYYYYKK..",
        ".KYYYYYYYYYYYYK.",
        ".KYYKKYYYYKKYYK.",
        "KYYYKKYYYYKKYYYK",
        "KYYYYYYYYYYYYYYK",
        "KYYYYYYYYYYYYYYK",
        "KYYYYYYYYYYYYYYK",
        "KYYYYRYYYYRYYYYK",
        "KYYYYRRRRRRYYYYK",
        "KYYYYYYYYYYYYYYK",
        ".KYYYYYYYYYYYYK.",
        ".KYYYYYYYYYYYYK.",
        "..KKYYYYYYYYKK..",
        "....KKKKKKKK....",
    };

    auto image = sf::Image::create({16u, 16u}, sf::Color::Transparent).value();

    for (unsigned int y = 0u; y < 16u; ++y)
        for (unsigned int x = 0u; x < 16u; ++x)
        {
            const char ch = pattern[y][x];

            if (ch == 'Y')
                image.setPixel({x, y}, {255u, 220u, 60u, 255u});
            else if (ch == 'K')
                image.setPixel({x, y}, {30u, 30u, 30u, 255u});
            else if (ch == 'R')
                image.setPixel({x, y}, {220u, 60u, 60u, 255u});
        }

    return image;
}

} // namespace


int main()
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size           = (resolution * 3.f).toVec2u(),
                          .title          = "SFML Pixel-Perfect",
                          .resizable      = true,
                          .vsync          = true,
                          .frametimeLimit = 144u,
                      })
                      .value();

    auto windowView = computePixelPerfectView(window.getSize().toVec2f(), resolution);
    auto worldView  = sf::View::fromScreenSize(resolution);

    auto rtGame = sf::RenderTexture::create(resolution.toVec2u(), {.antiAliasingLevel = 0u, .smooth = false}).value();

    const auto smileyImage   = makeSmileyImage();
    const auto smileyTexture = sf::Texture::loadFromImage(smileyImage, {.smooth = false}).value();

    sf::Vec2f spritePos{160.f, 120.f};
    sf::Vec2f spriteVel{47.f, 31.f}; // px/s, deliberately not axis-aligned

    sf::Clock clock;
    float     lastT = 0.f;

    while (true)
    {
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handlePixelPerfectResize(*event, resolution, windowView))
                continue;
        }

        const float t  = clock.getElapsedTime().asSeconds();
        const float dt = t - lastT;
        lastT          = t;

        // Bounce the smiley inside the backplate's interior:
        spritePos += spriteVel * dt;

        if (spritePos.x < 56.f)
        {
            spritePos.x = 56.f;
            spriteVel.x = -spriteVel.x;
        }
        else if (spritePos.x > 264.f)
        {
            spritePos.x = 264.f;
            spriteVel.x = -spriteVel.x;
        }
        if (spritePos.y < 66.f)
        {
            spritePos.y = 66.f;
            spriteVel.y = -spriteVel.y;
        }
        else if (spritePos.y > 174.f)
        {
            spritePos.y = 174.f;
            spriteVel.y = -spriteVel.y;
        }

        // Render the world at native resolution into the render texture:
        rtGame.clear({15u, 15u, 25u, 255u});

        rtGame.draw(
            sf::RectangleShapeData{
                .position         = {40.f, 50.f},
                .fillColor        = {30u, 60u, 90u, 255u},
                .outlineColor     = {180u, 200u, 220u, 255u},
                .outlineThickness = -1.f,
                .size             = {240.f, 140.f},
            },
            {.view = worldView});

        for (const sf::Vec2f cornerPos :
             {sf::Vec2f{40.f, 50.f}, sf::Vec2f{277.f, 50.f}, sf::Vec2f{40.f, 187.f}, sf::Vec2f{277.f, 187.f}})
        {
            rtGame.draw(
                sf::RectangleShapeData{
                    .position  = cornerPos.componentWiseFloor(),
                    .fillColor = {180u, 200u, 220u, 255u},
                    .size      = {3.f, 3.f},
                },
                {.view = worldView});
        }

        rtGame.draw(
            sf::CircleShapeData{
                .position   = sf::Vec2f{160.f, 120.f}.movedTowards(70.f, sf::radians(t)).componentWiseFloor(),
                .origin     = {6.f, 6.f},
                .rotation   = sf::radians(t),
                .fillColor  = {220u, 60u, 60u, 255u},
                .radius     = 6.f,
                .pointCount = 3u,
            },
            {.view = worldView});

        rtGame.draw(smileyTexture,
                    {
                        .position = spritePos.componentWiseFloor(),
                        .origin   = {8.f, 8.f},
                    },
                    {.view = worldView});

        rtGame.display();

        // Present the render texture through the pixel-perfect view:
        window.clear(sf::Color::Black);
        window.draw(rtGame.getTexture(), {.view = windowView});
        window.display();
    }

    return 0;
}
