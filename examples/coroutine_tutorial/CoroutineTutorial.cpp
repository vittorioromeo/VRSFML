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

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f worldSize{800.f, 600.f};


////////////////////////////////////////////////////////////
using sfex::Done;
using sfex::NextFrame;
using sfex::Wait;


////////////////////////////////////////////////////////////
struct Wall
{
    sf::Vec2f pos{}; // top-left
    sf::Vec2f size{};
};


////////////////////////////////////////////////////////////
struct Pickup
{
    sf::Vec2f pos{};
    bool      collected = false;
};


////////////////////////////////////////////////////////////
struct Door
{
    sf::Vec2f pos{}; // top-left
    sf::Vec2f size{};
    bool      open = false;
};


////////////////////////////////////////////////////////////
struct Player
{
    sf::Vec2f pos{}; // top-left
    sf::Vec2f size{20.f, 20.f};
    int       keys  = 0;
    float     speed = 140.f;
};


////////////////////////////////////////////////////////////
struct Guard
{
    sf::Vec2f pos{}; // top-left
    sf::Vec2f size{24.f, 24.f};

    sf::base::Vector<sf::Vec2f> waypoints; // top-left positions
    sf::base::SizeT             currentWaypointIdx = 0u;

    float detectionRadius = 95.f;
    float patrolSpeed     = 55.f;
    float chaseSpeed      = 95.f;

    enum class State
    {
        Patrolling,
        Suspicious,
        Chasing
    } state = State::Patrolling;
};


////////////////////////////////////////////////////////////
struct Dialogue
{
    bool             visible = false;
    sf::base::String fullText;
    sf::base::String displayedText;
};


////////////////////////////////////////////////////////////
struct World
{
    float dt = 0.f;

    Player player{};
    Guard  guard{};

    sf::base::Vector<Wall>   walls;
    sf::base::Vector<Pickup> keys;
    sf::base::Vector<Door>   doors;

    Dialogue         dialogue;
    sf::base::String objective;

    // game progress flags
    bool gameActive   = true;
    bool gameOver     = false;
    bool tutorialDone = false;

    // tutorial objective 1 progress
    bool movedUp    = false;
    bool movedDown  = false;
    bool movedLeft  = false;
    bool movedRight = false;

    // consumed by coroutines that wait for any key press
    bool keyEdgeQueued = false;
};


////////////////////////////////////////////////////////////
#define TUT_CO_WAIT_UNTIL(cond) SFEX_CO_WAIT_UNTIL(NextFrame{}, cond)


////////////////////////////////////////////////////////////
[[nodiscard]] bool aabbOverlap(sf::Vec2f aPos, sf::Vec2f aSize, sf::Vec2f bPos, sf::Vec2f bSize)
{
    return aPos.x < bPos.x + bSize.x && //
           aPos.x + aSize.x > bPos.x && //
           aPos.y < bPos.y + bSize.y && //
           aPos.y + aSize.y > bPos.y;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool playerInDetectionRadius(const World& world)
{
    return (world.player.pos - world.guard.pos).lengthSquared() <= world.guard.detectionRadius * world.guard.detectionRadius;
}


////////////////////////////////////////////////////////////
template <typename ObstacleRange, typename ShouldBlock>
void resolveCollisionsAxis(sf::Vec2f&     entityPos,
                           sf::Vec2f      entitySize,
                           ObstacleRange& obstacles,  // range of entities to check against
                           bool           xAxis,      // x or y axis?
                           float          delta,      // movement delta on chosen axis
                           ShouldBlock&&  shouldBlock // also used to open doors
)
{
    for (auto& o : obstacles)
    {
        if (!aabbOverlap(entityPos, entitySize, o.pos, o.size))
            continue;

        if (!shouldBlock(o))
            continue;

        if (xAxis)
        {
            if (delta > 0.f)
                entityPos.x = o.pos.x - entitySize.x;
            else if (delta < 0.f)
                entityPos.x = o.pos.x + o.size.x;
        }
        else
        {
            if (delta > 0.f)
                entityPos.y = o.pos.y - entitySize.y;
            else if (delta < 0.f)
                entityPos.y = o.pos.y + o.size.y;
        }
    }
}


////////////////////////////////////////////////////////////
void resolvePlayerMoveAxis(World& world, bool xAxis, float delta)
{
    Player& p = world.player;

    if (xAxis)
        p.pos.x += delta;
    else
        p.pos.y += delta;

    resolveCollisionsAxis(p.pos,
                          p.size,
                          world.walls,
                          xAxis,
                          delta,                             //
                          [](const Wall&) { return true; }); // walls are always solid

    resolveCollisionsAxis(p.pos,
                          p.size,
                          world.doors,
                          xAxis,
                          delta, //
                          [&](Door& d)
    {
        if (d.open)
            return false;

        if (p.keys >= 1)
        {
            d.open = true;
            p.keys -= 1;

            return false; // door just opened -- no collision
        }

        return true; // still locked, block
    });
}


////////////////////////////////////////////////////////////
void resolveGuardMoveAxis(World& world, bool xAxis, float delta)
{
    Guard& g = world.guard;

    if (xAxis)
        g.pos.x += delta;
    else
        g.pos.y += delta;

    resolveCollisionsAxis(g.pos,
                          g.size,
                          world.walls,
                          xAxis,
                          delta,                             //
                          [](const Wall&) { return true; }); // walls are always solid
}


////////////////////////////////////////////////////////////
void initLevel(World& world)
{
    world = {};

    world.walls.pushBackMultiple(
        // outer walls (frame)
        Wall{.pos = {0.f, 0.f}, .size = {worldSize.x, 30.f}},                // top
        Wall{.pos = {0.f, worldSize.y - 30.f}, .size = {worldSize.x, 30.f}}, // bottom
        Wall{.pos = {0.f, 0.f}, .size = {30.f, worldSize.y}},                // left
        Wall{.pos = {worldSize.x - 30.f, 0.f}, .size = {30.f, worldSize.y}}, // right

        // wall between room 1 (movement) and room 2 (key) with a passage in the middle
        Wall{.pos = {260.f, 30.f}, .size = {16.f, 200.f}},  // upper segment
        Wall{.pos = {260.f, 360.f}, .size = {16.f, 210.f}}, // lower segment

        // wall between room 2 (key) and room 3 (guard) with a passage in the middle
        Wall{.pos = {490.f, 30.f}, .size = {16.f, 240.f}},   // upper segment
        Wall{.pos = {490.f, 350.f}, .size = {16.f, 220.f}}); // lower segment

    // locked door between room 2 and room 3
    world.doors.emplaceBack(Door{.pos = {490.f, 270.f}, .size = {16.f, 80.f}});

    // pickup key sitting in room 2
    world.keys.emplaceBack(Pickup{.pos = {380.f, 300.f}});

    // player starts in room 1
    world.player.pos = {110.f, 290.f};

    // guard patrols vertically in room 3
    world.guard.pos = {588.f, 238.f};
    world.guard.waypoints.emplaceBack(sf::Vec2f{588.f, 88.f});
    world.guard.waypoints.emplaceBack(sf::Vec2f{588.f, 468.f});
    world.guard.currentWaypointIdx = 0u;
    world.guard.state              = Guard::State::Patrolling;
}


////////////////////////////////////////////////////////////
struct Typewriter : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    sf::base::SizeT charIdx = 0u; // persistent across yields


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        for (charIdx = 0u; charIdx < world.dialogue.fullText.size(); ++charIdx)
        {
            // fast-forward on key press
            if (world.keyEdgeQueued)
            {
                world.dialogue.displayedText = world.dialogue.fullText;
                world.keyEdgeQueued          = false;
                break;
            }

            world.dialogue.displayedText += world.dialogue.fullText[charIdx];
            SFEX_CO_YIELD(Wait{0.01f});
        }

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct DialogueBeat : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    sf::base::String text;
    Typewriter       typewriter;


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        // open the dialogue box and pause the game
        world.dialogue.fullText = text;
        world.dialogue.displayedText.clear();
        world.dialogue.visible = true;

        world.gameActive    = false;
        world.keyEdgeQueued = false; // ignore any latent press

        // type the text via sub-coroutine
        typewriter = {};
        SFEX_CO_AWAIT(typewriter(world));

        // drop any press used for fast-forwarding, then wait for a key press to dismiss
        world.keyEdgeQueued = false;
        TUT_CO_WAIT_UNTIL(world.keyEdgeQueued);
        world.keyEdgeQueued = false;

        // hide dialogue box and resume the game
        world.dialogue.visible = false;
        world.gameActive       = true;

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct GuardPatrol : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    sf::Vec2f delta;            // difference to next waypoint
    float     dist       = 0.f; // distance to next waypoint
    float     pauseTimer = 0.f; // pause timer at waypoint


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        while (true)
        {
            if (playerInDetectionRadius(world))
                SFEX_CO_RETURN(Done{});

            delta = world.guard.waypoints[world.guard.currentWaypointIdx] - world.guard.pos;
            dist  = delta.length();

            if (dist < 3.f)
            {
                // Reached the waypoint -- advance and pause briefly. The pause
                // is implemented as a manual NextFrame loop instead of a
                // single `Wait{0.6f}` so the radius check stays responsive.
                world.guard.currentWaypointIdx = (world.guard.currentWaypointIdx + 1u) % world.guard.waypoints.size();
                pauseTimer                     = 0.6f;

                while (pauseTimer > 0.f)
                {
                    if (playerInDetectionRadius(world))
                        SFEX_CO_RETURN(Done{});

                    pauseTimer -= world.dt;
                    SFEX_CO_YIELD(NextFrame{});
                }
            }
            else
            {
                // scope to avoid "crosses initialization with case label" error
                {
                    const sf::Vec2f dir  = delta / dist;
                    const sf::Vec2f step = dir * world.guard.patrolSpeed * world.dt;

                    resolveGuardMoveAxis(world, true, step.x);
                    resolveGuardMoveAxis(world, false, step.y);
                }

                SFEX_CO_YIELD(NextFrame{});
            }
        }

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct GuardSuspicious : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    float graceTimer         = 0.f;
    bool  confirmedDetection = false;


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        confirmedDetection = false;
        graceTimer         = 1.4f;
        world.guard.state  = Guard::State::Suspicious;

        while (graceTimer > 0.f)
        {
            if (!playerInDetectionRadius(world))
            {
                world.guard.state = Guard::State::Patrolling;
                SFEX_CO_RETURN(Done{});
            }

            graceTimer -= world.dt;
            SFEX_CO_YIELD(NextFrame{});
        }

        confirmedDetection = true;
        SFEX_CO_RETURN(Done{});

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct GuardChase : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    float lostTimer = 0.f;


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        world.guard.state = Guard::State::Chasing;
        lostTimer         = 0.f;

        while (true)
        {
            if (world.gameOver)
                SFEX_CO_RETURN(Done{});

            if (playerInDetectionRadius(world))
                lostTimer = 0.f;
            else
                lostTimer += world.dt;

            if (lostTimer >= 1.5f)
            {
                world.guard.state = Guard::State::Patrolling;
                SFEX_CO_RETURN(Done{});
            }

            // scope to avoid "crosses initialization with case label" error
            {
                const auto  delta = world.player.pos - world.guard.pos;
                const float dist  = delta.length();

                if (dist > 0.001f)
                {
                    const sf::Vec2f dir  = delta / dist;
                    const sf::Vec2f step = dir * world.guard.chaseSpeed * world.dt;

                    resolveGuardMoveAxis(world, /* xAxis */ true, step.x);
                    resolveGuardMoveAxis(world, /* xAxis */ false, step.y);
                }
            }

            SFEX_CO_YIELD(NextFrame{});
        }

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct GuardAI : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    GuardPatrol     patrol;
    GuardSuspicious suspicious;
    GuardChase      chase;


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        while (true)
        {
            patrol = {};
            SFEX_CO_AWAIT(patrol(world));

            suspicious = {};
            SFEX_CO_AWAIT(suspicious(world));

            if (suspicious.confirmedDetection)
            {
                chase = {};
                SFEX_CO_AWAIT(chase(world));

                if (world.gameOver)
                    SFEX_CO_RETURN(Done{});
            }
        }

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct TutorialDirector : sfex::Coroutine
{
    ////////////////////////////////////////////////////////////
    DialogueBeat dialogue;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool phaseOneSatisfied(const World& world)
    {
        const bool allArrows  = world.movedUp && world.movedDown && world.movedLeft && world.movedRight;
        const bool grabbedKey = world.player.keys >= 1;

        return allArrows || grabbedKey;
    }


    ////////////////////////////////////////////////////////////
    sfex::Yield operator()(World& world)
    {
        SFEX_CO_BEGIN;

        //
        // tutorial phase 1: movement (wait for all arrows or key pickup, whichever comes first)
        dialogue.text =
            "Welcome, agent.\n\nUse the arrow keys to move. To begin, please tap each "
            "of the four directions at least once.\n\nPress any key to continue.";
        SFEX_CO_AWAIT(dialogue(world));

        world.objective = "Tap each arrow key";
        TUT_CO_WAIT_UNTIL(phaseOneSatisfied(world));

        if (world.player.keys >= 1)
        {
            //
            // skip movement and key objectives if the player already picked up the key
            dialogue.text =
                "Hmm, looks like you've already found the key.\n"
                "Resourceful! Let's skip ahead.";
            SFEX_CO_AWAIT(dialogue(world));
        }
        else
        {
            //
            // tutorial phase 2: key pickup
            dialogue.text = "Good. There is a key in the next room. Walk over it to pick it up.";
            SFEX_CO_AWAIT(dialogue(world));

            world.objective = "Pick up the key";
            TUT_CO_WAIT_UNTIL(world.player.keys >= 1);
        }

        //
        // tutorial phase 3: open door
        dialogue.text =
            "Excellent. Use the key by walking into the locked door at the right of "
            "the room.\nThe door will consume the key as it opens.";
        SFEX_CO_AWAIT(dialogue(world));

        world.objective = "Unlock the door";
        TUT_CO_WAIT_UNTIL(world.doors.empty() ? false : world.doors[0].open);

        //
        // tutorial phase 4: guard avoidance
        dialogue.text =
            "There is a guard patrolling beyond. Stay outside the red detection circle.\n"
            "If the guard sees you it will pause briefly, then chase.\nReach the far "
            "right side of the corridor undetected.";
        SFEX_CO_AWAIT(dialogue(world));

        world.objective = "Reach the exit unseen";
        TUT_CO_WAIT_UNTIL(world.player.pos.x + world.player.size.x >= worldSize.x - 40.f);

        //
        // tutorial complete
        dialogue.text = "Well done, agent. The mission begins now.";
        SFEX_CO_AWAIT(dialogue(world));

        world.objective.clear();
        world.tutorialDone = true;

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct GameState
{
    World            world;
    TutorialDirector director;
    GuardAI          guardAI;
    float            directorWaitTimer = 0.f;
    float            guardWaitTimer    = 0.f;
    bool             directorDone      = false;
    bool             guardDone         = false;
};


////////////////////////////////////////////////////////////
void driveCoroutine(auto& coro, World& world, float& waitTimer, bool& done)
{
    if (done)
        return;

    if (waitTimer > 0.f)
    {
        waitTimer = sf::base::max(waitTimer - world.dt, 0.f);
        return;
    }

    coro(world).recursiveMatch( //
        [&](NextFrame) {},
        [&](const Wait& w) { waitTimer = w.seconds; },
        [&](Done) { done = true; });
}


////////////////////////////////////////////////////////////
void drawWorld(sf::RenderWindow&       window,
               const sf::Font&         font,
               const World&            world,
               const sf::base::String& statusMessage,
               float                   statusMessageAlpha)
{
    window.clear({18u, 22u, 30u});

    for (const Wall& w : world.walls)
        window.draw(sf::RectangleShapeData{
            .position  = w.pos,
            .fillColor = {70u, 80u, 105u},
            .size      = w.size,
        });

    for (const Door& d : world.doors)
    {
        if (d.open)
            continue;

        window.draw(sf::RectangleShapeData{
            .position         = d.pos,
            .fillColor        = {180u, 90u, 60u},
            .outlineColor     = {255u, 200u, 140u},
            .outlineThickness = 2.f,
            .size             = d.size,
        });
    }

    for (const Pickup& k : world.keys)
    {
        if (k.collected)
            continue;

        window.draw(sf::CircleShapeData{
            .position         = k.pos,
            .origin           = {7.f, 7.f},
            .fillColor        = {255u, 220u, 80u},
            .outlineColor     = {120u, 90u, 30u},
            .outlineThickness = 2.f,
            .radius           = 7.f,
        });
    }

    const auto guardRadiusColor = (world.guard.state == Guard::State::Suspicious) ? sf::Color{255u, 220u, 80u, 70u}
                                  : (world.guard.state == Guard::State::Chasing)  ? sf::Color{255u, 80u, 80u, 90u}
                                                                                  : sf::Color{120u, 200u, 255u, 40u};

    const auto guardBodyColor = (world.guard.state == Guard::State::Suspicious) ? sf::Color{255u, 220u, 80u}
                                : (world.guard.state == Guard::State::Chasing)  ? sf::Color{255u, 80u, 80u}
                                                                                : sf::Color{120u, 200u, 255u};

    window.draw(sf::CircleShapeData{
        .position         = world.guard.pos + world.guard.size * 0.5f,
        .origin           = {world.guard.detectionRadius, world.guard.detectionRadius},
        .fillColor        = guardRadiusColor,
        .outlineColor     = guardRadiusColor.withAlpha(200u),
        .outlineThickness = 1.f,
        .radius           = world.guard.detectionRadius,
    });

    window.draw(sf::RectangleShapeData{
        .position         = world.guard.pos,
        .fillColor        = guardBodyColor,
        .outlineColor     = sf::Color::White,
        .outlineThickness = 1.f,
        .size             = world.guard.size,
    });

    window.draw(sf::RectangleShapeData{
        .position         = world.player.pos,
        .fillColor        = {200u, 240u, 200u},
        .outlineColor     = sf::Color::Black,
        .outlineThickness = 1.f,
        .size             = world.player.size,
    });

    window.draw(font,
                sf::TextData{
                    .position         = {44.f, 38.f},
                    .string           = sf::base::String{"Keys: "} + static_cast<char>('0' + world.player.keys),
                    .characterSize    = 14u,
                    .fillColor        = {220u, 220u, 220u},
                    .outlineColor     = sf::Color::Black,
                    .outlineThickness = 1.f,
                });

    if (!world.objective.empty())
    {
        constexpr float boxW = 240.f;
        constexpr float boxH = 42.f;
        const float     boxX = worldSize.x - boxW - 24.f;
        const float     boxY = 24.f;

        window.draw(sf::RectangleShapeData{
            .position         = {boxX, boxY},
            .fillColor        = {30u, 40u, 60u, 200u},
            .outlineColor     = {180u, 220u, 255u},
            .outlineThickness = 1.f,
            .size             = {boxW, boxH},
        });

        window.draw(font,
                    sf::TextData{
                        .position         = {boxX + 10.f, boxY + 4.f},
                        .string           = "Objective",
                        .characterSize    = 11u,
                        .fillColor        = {180u, 220u, 255u},
                        .outlineColor     = sf::Color::Black,
                        .outlineThickness = 1.f,
                    });

        window.draw(font,
                    sf::TextData{
                        .position         = {boxX + 10.f, boxY + 18.f},
                        .string           = world.objective,
                        .characterSize    = 15u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::Black,
                        .outlineThickness = 1.f,
                    });
    }

    if (world.dialogue.visible)
    {
        constexpr float boxX = 40.f;
        constexpr float boxY = worldSize.y - 170.f;
        constexpr float boxW = worldSize.x - 80.f;
        constexpr float boxH = 130.f;

        window.draw(sf::RectangleShapeData{
            .position         = {boxX, boxY},
            .fillColor        = {16u, 24u, 48u, 235u},
            .outlineColor     = {180u, 220u, 255u},
            .outlineThickness = 2.f,
            .size             = {boxW, boxH},
        });

        window.draw(font,
                    sf::TextData{
                        .position         = {boxX + 16.f, boxY + 14.f},
                        .string           = world.dialogue.displayedText,
                        .characterSize    = 17u,
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::Black,
                        .outlineThickness = 1.f,
                    });

        window.draw(font,
                    sf::TextUtils::anchored(font,
                                            sf::TextData{
                                                .position         = {boxX + boxW - 8.f, boxY + boxH - 8.f},
                                                .string           = "[any key to continue]",
                                                .characterSize    = 11u,
                                                .fillColor        = {180u, 220u, 255u},
                                                .outlineColor     = sf::Color::Black,
                                                .outlineThickness = 1.f,
                                            },
                                            {1.f, 1.f}));
    }

    if (statusMessageAlpha > 0.f && !statusMessage.empty())
    {
        const auto alpha = static_cast<sf::base::U8>(sf::base::min(statusMessageAlpha, 1.f) * 255.f);
        window.draw(font,
                    sf::TextData{
                        .position         = {44.f, worldSize.y - 32.f},
                        .string           = statusMessage,
                        .characterSize    = 13u,
                        .fillColor        = {255u, 240u, 180u, alpha},
                        .outlineColor     = {0u, 0u, 0u, alpha},
                        .outlineThickness = 1.f,
                    });
    }

    window.draw(font,
                sf::TextData{
                    .position         = {worldSize.x - 360.f, worldSize.y - 18.f},
                    .string           = "Arrows move  R restart  F5 save  F6 load",
                    .characterSize    = 11u,
                    .fillColor        = {160u, 180u, 200u},
                    .outlineColor     = sf::Color::Black,
                    .outlineThickness = 1.f,
                });

    if (world.gameOver)
    {
        window.draw(sf::RectangleShapeData{
            .position  = {0.f, worldSize.y * 0.5f - 50.f},
            .fillColor = {0u, 0u, 0u, 200u},
            .size      = {worldSize.x, 100.f},
        });

        window.draw(font,
                    sf::TextUtils::anchored(font,
                                            {
                                                .position         = (worldSize / 2.f).addY(-20.f),
                                                .string           = "CAUGHT!",
                                                .characterSize    = 36u,
                                                .fillColor        = {255u, 100u, 100u},
                                                .outlineColor     = sf::Color::Black,
                                                .outlineThickness = 2.f,
                                                .bold             = true,
                                            },
                                            {0.5f, 0.5f}));

        window.draw(font,
                    sf::TextUtils::anchored(font,
                                            {
                                                .position         = (worldSize / 2.f).addY(20.f),
                                                .string           = "Press R to restart",
                                                .characterSize    = 16u,
                                                .fillColor        = sf::Color::White,
                                                .outlineColor     = sf::Color::Black,
                                                .outlineThickness = 1.f,
                                            },
                                            {0.5f, 0.5f}));
    }

    window.display();
}

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
                          .size             = worldSize.toVec2u(),
                          .title            = "SFEX Coroutine Tutorial",
                          .resizable        = false,
                          .vsync            = true,
                          .keyRepeatEnabled = false,
                      })
                      .value();

    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    GameState                     gs;
    sf::base::Optional<GameState> quickSave;

    sf::base::String statusMessage;
    float            statusMessageT  = 0.f;
    constexpr float  statusFlashTime = 1.6f;

    const auto flashStatus = [&](const char* msg)
    {
        statusMessage  = msg;
        statusMessageT = statusFlashTime;
    };

    const auto doRestart = [&]
    {
        gs = GameState{};
        initLevel(gs.world);
        flashStatus("restarted");
    };

    initLevel(gs.world);

    sf::Clock frameClock;

    while (true)
    {
        bool requestRestart   = false;
        bool requestQuickSave = false;
        bool requestQuickLoad = false;

        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
            {
                if (kp->code == sf::Keyboard::Key::F5)
                    requestQuickSave = true;
                else if (kp->code == sf::Keyboard::Key::F6)
                    requestQuickLoad = true;
                else if (kp->code == sf::Keyboard::Key::R)
                    requestRestart = true;
                else
                    gs.world.keyEdgeQueued = true;
            }
        }

        const float dt = frameClock.restart().asSeconds();
        gs.world.dt    = dt;

        if (requestRestart)
            doRestart();

        if (requestQuickSave)
        {
            quickSave.emplace(gs);
            flashStatus("quick-saved (F5)");
        }

        if (requestQuickLoad)
        {
            if (quickSave.hasValue())
            {
                gs = *quickSave;
                flashStatus("quick-loaded (F6)");
            }
            else
            {
                flashStatus("no quick-save to load");
            }
        }

        if (statusMessageT > 0.f)
            statusMessageT = sf::base::max(statusMessageT - dt, 0.f);

        if (gs.world.gameActive && !gs.world.gameOver)
        {
            const bool kUp    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
            const bool kDown  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
            const bool kLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
            const bool kRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);

            sf::Vec2f move{0.f, 0.f};

            // clang-format off
            if (kUp)    { move.y -= 1.f; gs.world.movedUp = true; }
            if (kDown)  { move.y += 1.f; gs.world.movedDown = true; }
            if (kLeft)  { move.x -= 1.f; gs.world.movedLeft = true; }
            if (kRight) { move.x += 1.f; gs.world.movedRight = true; }
            // clang-format on

            if (move.x != 0.f && move.y != 0.f)
                move *= 0.70710678f; // diagonal normalization

            const sf::Vec2f step = move * gs.world.player.speed * dt;
            resolvePlayerMoveAxis(gs.world, /* xAxis */ true, step.x);
            resolvePlayerMoveAxis(gs.world, /* xAxis */ false, step.y);

            for (Pickup& k : gs.world.keys)
            {
                if (k.collected)
                    continue;

                const sf::Vec2f playerCenter = gs.world.player.pos + gs.world.player.size * 0.5f;
                const sf::Vec2f delta        = playerCenter - k.pos;

                const float d2 = delta.x * delta.x + delta.y * delta.y;

                if (d2 <= 14.f * 14.f)
                {
                    k.collected = true;
                    gs.world.player.keys += 1;
                }
            }

            // capture check (player vs guard)
            if (aabbOverlap(gs.world.player.pos, gs.world.player.size, gs.world.guard.pos, gs.world.guard.size))
                gs.world.gameOver = true;
        }

        driveCoroutine(gs.director, gs.world, gs.directorWaitTimer, gs.directorDone);

        if (gs.world.gameActive && !gs.world.gameOver)
            driveCoroutine(gs.guardAI, gs.world, gs.guardWaitTimer, gs.guardDone);

        drawWorld(window, font, gs.world, statusMessage, statusMessageT / statusFlashTime);
    }
}
