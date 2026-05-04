////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/SfexCoroutine.hpp"
#include "ExampleUtils/SfexYield.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"

#include "SFML/Window/Event.hpp" // IWYU pragma: keep
#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f worldSize{800.f, 600.f};


////////////////////////////////////////////////////////////
using sfex::Done;
using sfex::NextFrame;
using sfex::Wait;


////////////////////////////////////////////////////////////
struct World
{
    ////////////////////////////////////////////////////////////
    // per-frame delta time in seconds
    float dt = 0.f;

    ////////////////////////////////////////////////////////////
    // dialogue box state
    const char*      speaker = "";
    sf::base::String text;
    bool             boxVisible = false;

    ////////////////////////////////////////////////////////////
    // character state
    sf::Vec2f heroPos;
    sf::Vec2f villainPos;
    bool      charactersVisible = false;


    ////////////////////////////////////////////////////////////
    void showText(const char* who, const char* what)
    {
        speaker    = who;
        text       = what;
        boxVisible = true;
    }


    ////////////////////////////////////////////////////////////
    void hideText()
    {
        boxVisible = false;
    }
};


////////////////////////////////////////////////////////////
struct Cutscene : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    // persistent "locals"
    int       paceIdx = 0;
    float     t       = 0.f;
    sf::Vec2f paceStartPos;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        world.heroPos           = {200.f, 280.f};
        world.villainPos        = {700.f, 280.f};
        world.charactersVisible = true;

        SFEX_CO_YIELD(Wait{0.5f});

        world.showText("Hero", "I finally got you.");
        SFEX_CO_YIELD(Wait{1.5f});

        world.hideText();
        SFEX_CO_YIELD(Wait{0.3f});

        world.showText("Villain", "You're too late!");
        SFEX_CO_YIELD(Wait{2.0f});

        world.hideText();
        SFEX_CO_YIELD(Wait{0.3f});

        for (paceIdx = 0; paceIdx < 3; ++paceIdx) // take three steps forward
        {
            paceStartPos = world.villainPos;
            t            = 0.f;

            while (t < 1.f) // smoothly move by 100px per step
            {
                t                  = sf::base::min(t + world.dt * 3.f, 1.f);
                world.villainPos.x = paceStartPos.x - 100.f * t;

                SFEX_CO_YIELD(NextFrame{});
            }

            SFEX_CO_YIELD(Wait{0.30f}); // menacing pause between paces
        }

        world.showText("Hero", "Don't come any closer!");
        SFEX_CO_YIELD(Wait{1.5f});

        world.hideText();
        SFEX_CO_YIELD(Wait{0.3f});

        paceStartPos = world.heroPos;
        t            = 0.f;

        while (t < 1.f)
        {
            t               = sf::base::min(t + world.dt * 3.f, 1.f);
            world.heroPos.x = paceStartPos.x - 60.f * t;

            SFEX_CO_YIELD(NextFrame{});
        }

        SFEX_CO_YIELD(Wait{2.f});

        world.charactersVisible = false;
        SFEX_CO_RETURN(Done{});

        SFEX_CO_END;
    }
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    auto window = sf::RenderWindow::create(
                      {
                          .size      = worldSize.toVec2u(),
                          .title     = "SFEX Coroutine Dialogue",
                          .resizable = false,
                          .vsync     = true,
                      })
                      .value();

    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    World    world;
    Cutscene scene;

    float waitTimer = 0.f;
    float restartIn = 0.f; // small pause between cutscene loops

    sf::Clock frameClock;

    while (true)
    {
        // -- event phase --

        while (const sf::base::Optional event = window.pollEvent())
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

        // -- update phase --

        world.dt = frameClock.restart().asSeconds();

        if (restartIn > 0.f)
        {
            restartIn -= world.dt;

            if (restartIn <= 0.f)
            {
                world     = {};
                scene     = {};
                waitTimer = 0.f;
            }
        }
        else if (waitTimer > 0.f)
        {
            waitTimer = sf::base::max(waitTimer - world.dt, 0.f);
        }
        else
        {
            scene(world).recursiveMatch( //
                [&](NextFrame) { /* run again next frame */ },
                [&](const Wait& w) { waitTimer = w.seconds; },
                [&](Done) { restartIn = 1.5f; });
        }

        // -- draw phase --

        window.clear({18u, 14u, 28u});

        if (world.charactersVisible)
        {
            window.draw(sf::CircleShapeData{
                .position         = world.heroPos,
                .origin           = {32.f, 32.f},
                .fillColor        = {120u, 220u, 255u},
                .outlineColor     = sf::Color::White,
                .outlineThickness = 2.f,
                .radius           = 32.f,
            });

            window.draw(font,
                        sf::TextUtils::anchored(font,
                                                sf::TextData{
                                                    .position         = world.heroPos.addY(-48.f),
                                                    .string           = "Hero",
                                                    .characterSize    = 19u,
                                                    .fillColor        = {180u, 220u, 255u},
                                                    .outlineColor     = sf::Color::Black,
                                                    .outlineThickness = 1.f,
                                                },
                                                {0.5f, 0.5f}));

            window.draw(sf::CircleShapeData{
                .position         = world.villainPos,
                .origin           = {32.f, 32.f},
                .fillColor        = {255u, 100u, 100u},
                .outlineColor     = sf::Color::White,
                .outlineThickness = 2.f,
                .radius           = 32.f,
            });

            window.draw(font,
                        sf::TextUtils::anchored(font,
                                                sf::TextData{
                                                    .position         = world.villainPos.addY(-48.f),
                                                    .string           = "Villain",
                                                    .characterSize    = 19u,
                                                    .fillColor        = {255u, 180u, 180u},
                                                    .outlineColor     = sf::Color::Black,
                                                    .outlineThickness = 1.f,
                                                },
                                                {0.5f, 0.5f}));
        }

        if (world.boxVisible)
        {
            constexpr sf::Vec2f boxPos  = {40.f, worldSize.y - 180.f};
            constexpr sf::Vec2f boxSize = {worldSize.x - 80.f, 140.f};

            window.draw(sf::RectangleShapeData{
                .position         = boxPos,
                .fillColor        = {20u, 30u, 60u},
                .outlineColor     = {180u, 220u, 255u},
                .outlineThickness = 2.f,
                .size             = boxSize,
            });

            window.draw(font,
                        sf::TextData{
                            .position         = boxPos + sf::Vec2f{12.f, -28.f},
                            .string           = world.speaker,
                            .characterSize    = 18u,
                            .fillColor        = {255u, 240u, 120u},
                            .outlineColor     = sf::Color::Black,
                            .outlineThickness = 2.f,
                            .bold             = true,
                        });

            window.draw(font,
                        sf::TextData{
                            .position         = boxPos + sf::Vec2f{16.f, 16.f},
                            .string           = world.text,
                            .characterSize    = 22u,
                            .fillColor        = sf::Color::White,
                            .outlineColor     = sf::Color::Black,
                            .outlineThickness = 1.f,
                        });
        }

        window.display();
    }
}
