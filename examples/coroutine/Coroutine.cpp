////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Scaling.hpp"
#include "ExampleUtils/SfexCoroutine.hpp"
#include "ExampleUtils/SfexYield.hpp"

#include "Zancle/Graphics/CircleShapeData.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/TextData.hpp"
#include "Zancle/Graphics/TextUtils.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/Keyboard.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Vec2.hpp"

#include "ZancleBase/Algorithm/SwapAndPop.hpp"
#include "ZancleBase/Clamp.hpp"
#include "ZancleBase/Constants.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/MinMax.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToString.hpp"
#include "ZancleBase/Variant.hpp"
#include "ZancleBase/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
constexpr za::Vec2f worldSize{800.f, 600.f};


////////////////////////////////////////////////////////////
using sfex::Done;
using sfex::NextFrame;
using sfex::Wait;
using sfex::Yield;


////////////////////////////////////////////////////////////
template <typename T>
struct Tween
{
    ////////////////////////////////////////////////////////////
    float t = 0.f;

    T from{};
    T to{};


    ////////////////////////////////////////////////////////////
    void start(T fromValue, T toValue)
    {
        t = 0.f;

        from = fromValue;
        to   = toValue;
    }


    ////////////////////////////////////////////////////////////
    bool step(float dt, float duration)
    {
        if (t >= 1.f)
            return false;

        t = zb::min(t + dt / duration, 1.f);
        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T sample(auto&& easingFn) const
    {
        return blend(from, to, easingFn(t));
    }
};


////////////////////////////////////////////////////////////
struct Bullet
{
    za::Vec2f pos;
    za::Vec2f vel;
    float     radius = 6.f;
    za::Color color  = za::Color::White;
};


////////////////////////////////////////////////////////////
struct PlayerBullet
{
    za::Vec2f pos;
    za::Vec2f vel;
    float     radius = 4.f;
    bool      dead   = false;
};


////////////////////////////////////////////////////////////
struct Player
{
    za::Vec2f pos           = {worldSize.x * 0.5f, worldSize.y - 60.f};
    float     drawRadius    = 9.f;
    float     hitRadius     = 3.f;
    bool      alive         = true;
    float     shootCooldown = 0.f;
};


////////////////////////////////////////////////////////////
struct Boss
{
    ////////////////////////////////////////////////////////////
    // game values
    za::Vec2f homePos;
    za::Vec2f pos;
    za::Vec2f vel        = {0.f, 0.f};
    float     radius     = 30.f;
    float     scale      = 1.f;
    za::Color color      = {120u, 170u, 255u};
    za::Color bodyTint   = {120u, 170u, 255u};
    float     maxHp      = 120.f;
    float     hp         = 120.f;
    bool      alive      = true;
    float     deathTimer = 0.f;


    ////////////////////////////////////////////////////////////
    // per-boss coroutine scheduling
    float waitTimer = 0.f;
    float waitCarry = 0.f;
    float restartIn = 0.f;


    ////////////////////////////////////////////////////////////
    // flags to prevent conflicts between overlapping movement coroutines
    bool dodging    = false;
    bool busyMoving = false;
};


////////////////////////////////////////////////////////////
struct World
{
    ////////////////////////////////////////////////////////////
    float dt   = 0.f;
    float time = 0.f;


    ////////////////////////////////////////////////////////////
    Player player{};


    ////////////////////////////////////////////////////////////
    zb::Vector<Boss>         bosses;
    zb::Vector<Bullet>       bullets;
    zb::Vector<PlayerBullet> playerBullets;
    zb::Vector<zb::String>   log;


    ////////////////////////////////////////////////////////////
    static constexpr zb::SizeT maxLogLines = 22u;


    ////////////////////////////////////////////////////////////
    void addLog(const char* who, const zb::String& what)
    {
        zb::String line = "[t=";
        zb::appendToString(line, time);
        line += "s] ";
        line += who;
        line += "  ";
        line += what;

        log.emplaceBack(ZB_MOVE(line));

        if (log.size() > maxLogLines)
            log.erase(log.begin());
    }


    ////////////////////////////////////////////////////////////
    void spawnBulletRing(za::Vec2f source, int count, float speed, za::Angle startAngle, za::Color color, float bulletRadius)
    {
        const auto total = za::radians(zb::tau);
        for (int i = 0; i < count; ++i)
        {
            const za::Angle a = startAngle + total * (static_cast<float>(i) / static_cast<float>(count));
            bullets.emplaceBack(Bullet{
                .pos    = source,
                .vel    = za::Vec2f::fromAngle(speed, a),
                .radius = bulletRadius,
                .color  = color,
            });
        }
    }


    ////////////////////////////////////////////////////////////
    void spawnAimedBullet(za::Vec2f source, za::Vec2f target, float speed, za::Color color)
    {
        za::Vec2f   dir = target - source;
        const float len = dir.length();
        if (len > 0.0001f)
            dir /= len;

        bullets.emplaceBack(Bullet{
            .pos    = source,
            .vel    = dir * speed,
            .radius = 7.f,
            .color  = color,
        });
    }


    ////////////////////////////////////////////////////////////
    void initBosses()
    {
        bosses.clear();

        struct Spawn
        {
            za::Vec2f home{};
            za::Color tint{};
            float     maxHp = 0.f;
        };

        const Spawn spawns[]{
            {.home = {worldSize.x * 0.25f, 140.f}, .tint = {120u, 170u, 255u}, .maxHp = 100.f},
            // {.home = {worldSize.x * 0.50f, 100.f}, .tint = {255u, 180u, 120u}, .maxHp = 140.f},
            // {.home = {worldSize.x * 0.75f, 140.f}, .tint = {200u, 120u, 255u}, .maxHp = 100.f},
        };

        for (const Spawn& s : spawns)
            bosses.emplaceBack(Boss{
                .homePos  = s.home,
                .pos      = s.home,
                .color    = s.tint,
                .bodyTint = s.tint,
                .maxHp    = s.maxHp,
                .hp       = s.maxHp,
            });

        addLog("SYSTEM", "spawned " + zb::toString(bosses.size()) + " bosses");
    }
};


////////////////////////////////////////////////////////////
struct BossCtx
{
    World& world;
    Boss&  self;
};


////////////////////////////////////////////////////////////
// found via ADL by the coroutine system, used to apply yielded waits to the parallel child coroutines
void yieldApply(const Yield& y, auto& child)
{
    y.recursiveMatch( //
        [&](NextFrame) {},
        [&](const Wait& w) { child.parallelWait = w.seconds; },
        [&](Done) { child.parallelDone = true; });
}


////////////////////////////////////////////////////////////
struct BossCoroutine : sfex::Coroutine, sfex::CoroutineParallelMixin<float>
{
};


////////////////////////////////////////////////////////////
#define BOSS_CO_AWAIT_ALL(...)   SFEX_CO_AWAIT_ALL(ctx.world.dt, ctx, NextFrame{}, __VA_ARGS__)
#define BOSS_CO_AWAIT_ANY(...)   SFEX_CO_AWAIT_ANY(ctx.world.dt, ctx, NextFrame{}, __VA_ARGS__)
#define BOSS_CO_WAIT_UNTIL(cond) SFEX_CO_WAIT_UNTIL(NextFrame{}, cond)
#define BOSS_CO_WAIT_WHILE(cond) SFEX_CO_WAIT_WHILE(NextFrame{}, cond)


////////////////////////////////////////////////////////////
struct BulletRingBarrage : BossCoroutine
{
    int       ringsToFire    = 3;
    int       bulletsPerRing = 12;
    float     gapSeconds     = 0.30f;
    float     bulletSpeed    = 140.f;
    float     bulletRadius   = 6.f;
    za::Angle ringAngleStep  = za::degrees(7.f);
    za::Color bulletColor    = {255u, 180u, 80u};

    int       i         = 0;
    za::Angle ringAngle = za::Angle::Zero;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        ringAngle = za::Angle::Zero;
        for (i = 0; i < ringsToFire; ++i)
        {
            ctx.world.spawnBulletRing(ctx.self.pos, bulletsPerRing, bulletSpeed, ringAngle, bulletColor, bulletRadius);
            ringAngle += ringAngleStep;
            SFEX_CO_YIELD(Wait{gapSeconds});
        }

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct DashAttack : BossCoroutine
{
    za::Vec2f dashTarget;
    float     dashTime = 0.25f;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        BOSS_CO_WAIT_WHILE(ctx.self.dodging);
        ctx.self.busyMoving = true;

        ctx.self.color = {255u, 80u, 80u};
        SFEX_CO_YIELD(Wait{0.50f});

        ctx.self.color = {255u, 220u, 60u};
        SFEX_CO_YIELD(Wait{0.15f});

        ctx.self.vel = (dashTarget - ctx.self.pos) / dashTime;
        for (int k = -2; k <= 2; ++k)
        {
            const za::Angle a = ctx.self.vel.angle() + za::degrees(static_cast<float>(k) * 10.f);
            ctx.world.bullets.emplaceBack(Bullet{
                .pos    = ctx.self.pos,
                .vel    = za::Vec2f::fromAngle(220.f, a),
                .radius = 6.f,
                .color  = {255u, 140u, 40u},
            });
        }
        SFEX_CO_YIELD(Wait{dashTime});

        ctx.self.vel   = {0.f, 0.f};
        ctx.self.color = ctx.self.bodyTint;
        SFEX_CO_YIELD(Wait{0.40f});

        ctx.self.busyMoving = false;
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct AimedVolley : BossCoroutine
{
    int   shotsToFire = 5;
    float gapSeconds  = 0.12f;

    int j = 0;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        for (j = 0; j < shotsToFire; ++j)
        {
            ctx.world.spawnAimedBullet(ctx.self.pos, ctx.world.player.pos, 260.f, {255u, 120u, 200u});
            SFEX_CO_YIELD(Wait{gapSeconds});
        }

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct SlamAttack : BossCoroutine
{
    za::Vec2f windupOffset = {0.f, -80.f};
    za::Vec2f slamOffset   = {0.f, 180.f};

    Tween<za::Vec2f> tween{};

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        BOSS_CO_WAIT_WHILE(ctx.self.dodging);
        ctx.self.busyMoving = true;

        ctx.self.color = {255u, 220u, 60u};
        tween.start(ctx.self.pos, ctx.self.homePos + windupOffset);
        while (tween.step(ctx.world.dt, 0.50f))
        {
            ctx.self.pos = tween.sample(easeInOutCubic);
            SFEX_CO_YIELD(NextFrame{});
        }

        ctx.self.color = {255u, 80u, 80u};
        tween.start(ctx.self.pos, ctx.self.homePos + slamOffset);
        while (tween.step(ctx.world.dt, 0.12f))
        {
            ctx.self.pos = tween.sample(easeInQuint);
            SFEX_CO_YIELD(NextFrame{});
        }

        ctx.world.spawnBulletRing(ctx.self.pos, 18, 240.f, za::Angle::Zero, {255u, 140u, 40u}, 6.f);
        SFEX_CO_YIELD(Wait{0.15f});

        ctx.self.color = ctx.self.bodyTint;
        tween.start(ctx.self.pos, ctx.self.homePos);
        while (tween.step(ctx.world.dt, 0.50f))
        {
            ctx.self.pos = tween.sample(easeInOutSine);
            SFEX_CO_YIELD(NextFrame{});
        }

        ctx.self.busyMoving = false;
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct PulseAttack : BossCoroutine
{
    int   pulses      = 3;
    int   ringBullets = 14;
    float maxScale    = 1.55f;

    Tween<float> scaleTween{};
    int          p = 0;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        for (p = 0; p < pulses; ++p)
        {
            ctx.self.color = {240u, 200u, 120u};
            scaleTween.start(1.f, maxScale);
            while (scaleTween.step(ctx.world.dt, 0.55f))
            {
                ctx.self.scale = scaleTween.sample(easeOutElastic);
                SFEX_CO_YIELD(NextFrame{});
            }

            ctx.self.color = {255u, 120u, 60u};
            ctx.world.spawnBulletRing(ctx.self.pos,
                                      ringBullets,
                                      160.f,
                                      za::radians(static_cast<float>(p) * 0.13f),
                                      {255u, 200u, 120u},
                                      6.f);

            scaleTween.start(maxScale, 1.f);
            while (scaleTween.step(ctx.world.dt, 0.18f))
            {
                ctx.self.scale = scaleTween.sample(easeInQuint);
                SFEX_CO_YIELD(NextFrame{});
            }
        }

        ctx.self.color = ctx.self.bodyTint;
        ctx.self.scale = 1.f;

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct SweepAttack : BossCoroutine
{
    float sweepRange    = 120.f;
    float sweepDuration = 1.60f;
    float spawnEvery    = 0.10f;

    Tween<za::Vec2f> tween{};
    float            spawnTimer = 0.f;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        BOSS_CO_WAIT_WHILE(ctx.self.dodging);
        ctx.self.busyMoving = true;

        ctx.self.color = {180u, 140u, 255u};
        tween.start(ctx.self.pos, ctx.self.homePos + za::Vec2f{-sweepRange, 0.f});
        while (tween.step(ctx.world.dt, sweepDuration))
        {
            ctx.self.pos = tween.sample(easeInOutQuint);
            SFEX_CO_YIELD(NextFrame{});
        }

        tween.start(ctx.self.pos, ctx.self.homePos + za::Vec2f{sweepRange, 0.f});
        spawnTimer = 0.f;
        while (tween.step(ctx.world.dt, sweepDuration))
        {
            spawnTimer += ctx.world.dt;

            ctx.self.pos = tween.sample(easeInOutBack);

            if (spawnTimer >= spawnEvery)
            {
                spawnTimer = 0.f;
                ctx.world.spawnAimedBullet(ctx.self.pos, ctx.world.player.pos, 240.f, {200u, 160u, 255u});
            }
            SFEX_CO_YIELD(NextFrame{});
        }

        ctx.self.color      = ctx.self.bodyTint;
        ctx.self.busyMoving = false;
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct Timer : BossCoroutine
{
    float duration = 1.f;

    Yield operator()(BossCtx)
    {
        SFEX_CO_BEGIN;
        SFEX_CO_YIELD(Wait{duration});
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct DodgeWatcher : BossCoroutine
{
    float xThreshold = 45.f;

    [[nodiscard]] bool playerIsUnderneath(BossCtx ctx) const
    {
        const float dx = zb::fabs(ctx.world.player.pos.x - ctx.self.pos.x);
        return ctx.world.player.alive && !ctx.self.busyMoving && (dx <= xThreshold);
    }

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        BOSS_CO_WAIT_UNTIL(playerIsUnderneath(ctx));
        SFEX_CO_RETURN(Done{});

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct DodgeMove : BossCoroutine
{
    Tween<za::Vec2f> tween{};

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;
        ctx.self.dodging = true;
        ctx.self.color   = {255u, 255u, 100u}; // flash yellow

        tween.start(ctx.self.pos, ctx.self.pos + za::Vec2f{ctx.world.player.pos.x > ctx.self.pos.x ? 135.f : -135.f, 0.f});
        while (tween.step(ctx.world.dt, 0.25f))
        {
            ctx.self.pos = tween.sample(easeOutQuint);
            SFEX_CO_YIELD(NextFrame{});
        }

        ctx.self.color   = ctx.self.bodyTint;
        ctx.self.dodging = false;
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct DodgeSupervisor : BossCoroutine
{
    DodgeWatcher watcher{};
    DodgeMove    dodge{};

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        while (true)
        {
            watcher = {};
            SFEX_CO_AWAIT(watcher(ctx));

            ctx.world.addLog("BOSS", "player is underneath! dodging...");
            dodge = {};
            SFEX_CO_AWAIT(dodge(ctx));

            // Edge-trigger: don't re-arm until the player has cleared the
            // zone. Otherwise we'd dodge again next frame whenever the
            // player tracks with us.
            BOSS_CO_WAIT_WHILE(watcher.playerIsUnderneath(ctx));
        }

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct BossPhases : BossCoroutine
{
    BulletRingBarrage barrage;
    DashAttack        dash;
    AimedVolley       volley;
    SlamAttack        slam;
    PulseAttack       pulse;
    SweepAttack       sweep;
    Timer             timer;
    BulletRingBarrage parallelBarrage;
    SweepAttack       parallelSweep;
    AimedVolley       parallelVolley;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        // ----- PHASE 1: opening salvo -----
        barrage = BulletRingBarrage{
            .ringsToFire    = 2,
            .bulletsPerRing = 10,
            .gapSeconds     = 0.30f,
            .bulletColor    = {255u, 200u, 100u},
        };
        SFEX_CO_AWAIT(barrage(ctx));

        SFEX_CO_YIELD(Wait{0.50f});

        // ----- PHASE 2: elastic pulse -----
        pulse = PulseAttack{
            .pulses      = 2,
            .ringBullets = 12,
        };
        SFEX_CO_AWAIT(pulse(ctx));

        SFEX_CO_YIELD(Wait{0.40f});

        // ----- PHASE 3: sweep -----
        sweep = SweepAttack{};
        SFEX_CO_AWAIT(sweep(ctx));

        SFEX_CO_YIELD(Wait{0.30f});

        // ----- PHASE 3.5: parallel barrage + sweep -----
        parallelBarrage = BulletRingBarrage{
            .ringsToFire    = 4,
            .bulletsPerRing = 10,
            .gapSeconds     = 0.25f,
            .bulletSpeed    = 120.f,
            .bulletColor    = {150u, 220u, 255u},
        };
        parallelSweep = SweepAttack{
            .sweepDuration = 1.60f,
        };
        BOSS_CO_AWAIT_ALL(parallelBarrage, parallelSweep);

        SFEX_CO_YIELD(Wait{0.40f});

        // ----- PHASE 3.8: volley vs timeout -----
        parallelVolley = AimedVolley{
            .shotsToFire = 15,
            .gapSeconds  = 0.15f,
        };
        timer = Timer{
            .duration = 1.5f,
        };
        BOSS_CO_AWAIT_ANY(parallelVolley, timer);

        SFEX_CO_YIELD(Wait{0.40f});

        // ----- PHASE 4: double slam -----
        slam = SlamAttack{
            .slamOffset = {-60.f, 180.f},
        };
        SFEX_CO_AWAIT(slam(ctx));

        slam = SlamAttack{
            .slamOffset = {60.f, 180.f},
        };
        SFEX_CO_AWAIT(slam(ctx));

        // ----- PHASE 5: dash + vulnerability -----
        dash = DashAttack{
            .dashTarget = ctx.self.homePos + za::Vec2f{-80.f, 40.f},
        };
        SFEX_CO_AWAIT(dash(ctx));

        dash = DashAttack{
            .dashTarget = ctx.self.homePos + za::Vec2f{80.f, 40.f},
        };
        SFEX_CO_AWAIT(dash(ctx));

        // ----- PHASE 6: aimed volley -----
        volley = AimedVolley{
            .shotsToFire = 6,
        };
        SFEX_CO_AWAIT(volley(ctx));

        SFEX_CO_YIELD(Wait{0.40f});

        // ----- PHASE 7: desperation spiral -----
        barrage = BulletRingBarrage{
            .ringsToFire    = 6,
            .bulletsPerRing = 12,
            .gapSeconds     = 0.18f,
            .bulletSpeed    = 160.f,
            .ringAngleStep  = za::degrees(11.f),
            .bulletColor    = {255u, 120u, 120u},
        };
        SFEX_CO_AWAIT(barrage(ctx));

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct BossFight : BossCoroutine
{
    BossPhases      phases;
    DodgeSupervisor supervisor;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        BOSS_CO_AWAIT_ANY(phases, supervisor);
        SFEX_CO_RETURN(Done{});

        SFEX_CO_END;
    }
};


////////////////////////////////////////////////////////////
struct Snapshot
{
    World                 world;
    zb::Vector<BossFight> bossScripts;
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = worldSize.toVec2u(),
                          .title     = "Zancle Coroutine (macro-based boss fight)",
                          .resizable = true,
                          .vsync     = true,
                      })
                      .value();

    auto windowView = computeAspectRatioAwareView(window.getSize().toVec2f(), worldSize);

    const auto font = za::Font::openFromFile("resources/tuffy.ttf").value();

    World                 world;
    zb::Vector<BossFight> bossScripts; // parallel to `world.bosses`

    const auto resetScriptsToBosses = [&] { bossScripts.resize(world.bosses.size()); };

    world.initBosses();
    resetScriptsToBosses();

    za::Clock              frameClock;
    zb::Optional<Snapshot> quickSave;

    const auto doRestart = [&]
    {
        world = {};
        world.initBosses();

        bossScripts.clear();
        resetScriptsToBosses();

        world.addLog("SYSTEM", "=== RESTART ===");
    };

    while (true)
    {
        bool requestRestart   = false;
        bool requestQuickSave = false;
        bool requestQuickLoad = false;

        while (const zb::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, worldSize, windowView))
                continue;

            if (const auto* kp = event->getIf<za::Event::KeyPressed>())
            {
                if (kp->code == za::Keyboard::Key::R)
                    requestRestart = true;
                else if (kp->code == za::Keyboard::Key::F5)
                    requestQuickSave = true;
                else if (kp->code == za::Keyboard::Key::F6)
                    requestQuickLoad = true;
            }
        }

        if (requestRestart)
            doRestart();

        if (requestQuickSave)
        {
            quickSave.emplace(Snapshot{.world = world, .bossScripts = bossScripts});
            world.addLog("SYSTEM", "quick-saved (F5)");
        }

        if (requestQuickLoad)
        {
            if (quickSave.hasValue())
            {
                world       = quickSave->world;
                bossScripts = quickSave->bossScripts;
                world.addLog("SYSTEM", "quick-loaded (F6)");
            }
            else
            {
                world.addLog("SYSTEM", "no quick-save to load (F6)");
            }
        }

        const float dt = frameClock.restart().asSeconds();
        world.time += dt;
        world.dt = dt;

        zb::SizeT aliveCount = 0;
        for (const Boss& b : world.bosses)
            if (b.alive)
                ++aliveCount;

        const bool gameActive = world.player.alive && aliveCount > 0;

        // ---- Player input ----
        if (gameActive)
        {
            using K           = za::Keyboard::Key;
            const bool  focus = za::Keyboard::isKeyPressed(K::LShift);
            const float speed = focus ? 140.f : 320.f;

            za::Vec2f move{0.f, 0.f};

            if (za::Keyboard::isKeyPressed(K::Left))
                move.x -= 1.f;

            if (za::Keyboard::isKeyPressed(K::Right))
                move.x += 1.f;

            if (za::Keyboard::isKeyPressed(K::Up))
                move.y -= 1.f;

            if (za::Keyboard::isKeyPressed(K::Down))
                move.y += 1.f;

            if (move.lengthSquared() > 0.f)
            {
                const float invLen = 1.f / move.length();
                move *= invLen;
            }

            world.player.pos += move * (speed * dt);

            const float m      = world.player.drawRadius;
            world.player.pos.x = zb::clamp(world.player.pos.x, m, worldSize.x - m);
            world.player.pos.y = zb::clamp(world.player.pos.y, m, worldSize.y - m);

            world.player.shootCooldown = zb::max(world.player.shootCooldown - dt, 0.f);

            if (za::Keyboard::isKeyPressed(K::Z) && world.player.shootCooldown == 0.f)
            {
                world.player.shootCooldown = 0.07f;

                world.playerBullets.emplaceBack(PlayerBullet{
                    .pos = world.player.pos + za::Vec2f{-9.f, -6.f},
                    .vel = {0.f, -900.f},
                });

                world.playerBullets.emplaceBack(PlayerBullet{
                    .pos = world.player.pos + za::Vec2f{9.f, -6.f},
                    .vel = {0.f, -900.f},
                });
            }
        }

        // ---- Per-boss coroutine driver ----
        if (gameActive)
        {
            for (zb::SizeT i = 0u; i < world.bosses.size(); ++i)
            {
                Boss& boss = world.bosses[i];
                if (!boss.alive)
                    continue;

                BossCtx    ctx{world, boss};
                BossFight& script = bossScripts[i];

                if (boss.restartIn > 0.f)
                {
                    boss.restartIn = zb::max(boss.restartIn - dt, 0.f);
                    if (boss.restartIn == 0.f)
                    {
                        script          = BossFight{};
                        boss.waitCarry  = 0.f;
                        boss.dodging    = false;
                        boss.busyMoving = false;
                    }
                }
                else if (boss.waitTimer > 0.f)
                {
                    if (dt >= boss.waitTimer)
                    {
                        boss.waitCarry += dt - boss.waitTimer;
                        boss.waitTimer = 0.f;
                    }
                    else
                    {
                        boss.waitTimer -= dt;
                    }
                }
                else
                {
                    script(ctx).recursiveMatch( //
                        [&](NextFrame) {},
                        [&](const Wait& w)
                    {
                        boss.waitTimer = w.seconds - boss.waitCarry;
                        boss.waitCarry = 0.f;
                        if (boss.waitTimer < 0.f)
                        {
                            boss.waitCarry = -boss.waitTimer;
                            boss.waitTimer = 0.f;
                        }
                    },
                        [&](Done) { boss.restartIn = 2.f; });
                }
            }
        }

        // ---- Per-boss death transitions & position integration ----
        for (Boss& boss : world.bosses)
        {
            if (boss.alive && boss.hp <= 0.f)
            {
                boss.alive      = false;
                boss.deathTimer = 1.5f;
                boss.vel        = {0.f, 0.f};
                world.addLog("BOSS", "defeated!");
            }
            if (!boss.alive && boss.deathTimer > 0.f)
            {
                boss.deathTimer = zb::max(boss.deathTimer - dt, 0.f);
                const float k   = boss.deathTimer / 1.5f;
                boss.scale      = k;
                boss.color.a    = static_cast<zb::U8>(255.f * k);
            }
            if (boss.alive)
                boss.pos += boss.vel * dt;
        }

        static bool lastBossCleared = false;
        if (aliveCount == 0 && !lastBossCleared)
        {
            world.bullets.clear();
            lastBossCleared = true;
        }
        if (aliveCount > 0)
            lastBossCleared = false;

        // ---- Enemy bullets vs player ----
        zb::vectorSwapAndPopIf(world.bullets,
                               [&](Bullet& b)
        {
            b.pos += b.vel * dt;

            if (world.player.alive)
            {
                const float r  = b.radius + world.player.hitRadius;
                const auto  dd = (b.pos - world.player.pos).lengthSquared();
                if (dd < r * r)
                {
                    world.player.alive = false;
                    world.addLog("PLAYER", "hit!");
                }
            }

            return b.pos.x < -50.f || b.pos.x > worldSize.x + 50.f || //
                   b.pos.y < -50.f || b.pos.y > worldSize.y + 50.f;
        });

        // ---- Player bullets vs bosses ----
        zb::vectorSwapAndPopIf(world.playerBullets,
                               [&](PlayerBullet& b)
        {
            b.pos += b.vel * dt;

            if (!b.dead)
            {
                for (Boss& boss : world.bosses)
                {
                    if (!boss.alive)
                        continue;
                    const float r  = b.radius + boss.radius * boss.scale;
                    const auto  dd = (b.pos - boss.pos).lengthSquared();
                    if (dd < r * r)
                    {
                        boss.hp = zb::max(boss.hp - 1.f, 0.f);
                        b.dead  = true;
                        break;
                    }
                }
            }

            return b.dead || b.pos.y < -20.f || b.pos.y > worldSize.y + 20.f || //
                   b.pos.x < -20.f || b.pos.x > worldSize.x + 20.f;
        });

        // ---- Draw ----
        window.clear({18u, 14u, 28u});

        const auto drawCtx = window.withRenderStates({.view = windowView});

        for (const Boss& boss : world.bosses)
        {
            if (!boss.alive && boss.deathTimer == 0.f)
                continue;

            const float bossDrawRadius = boss.radius * boss.scale;
            drawCtx.draw(za::CircleShapeData{
                .position         = boss.pos,
                .origin           = {bossDrawRadius, bossDrawRadius},
                .fillColor        = boss.color,
                .outlineColor     = za::Color::White,
                .outlineThickness = 2.f,
                .radius           = bossDrawRadius,
            });
        }

        for (const Bullet& b : world.bullets)
            drawCtx.draw(za::CircleShapeData{
                .position         = b.pos,
                .origin           = {b.radius, b.radius},
                .fillColor        = b.color,
                .outlineColor     = {0u, 0u, 0u, 180u},
                .outlineThickness = 1.f,
                .radius           = b.radius,
            });

        for (const PlayerBullet& b : world.playerBullets)
            drawCtx.draw(za::RectangleShapeData{
                .position         = b.pos,
                .origin           = {3.f, 7.f},
                .fillColor        = {180u, 240u, 255u},
                .outlineColor     = {40u, 120u, 200u},
                .outlineThickness = 1.f,
                .size             = {6.f, 14.f},
            });

        if (world.player.alive)
        {
            drawCtx.draw(za::CircleShapeData{
                .position         = world.player.pos,
                .origin           = {world.player.drawRadius, world.player.drawRadius},
                .fillColor        = {120u, 220u, 255u},
                .outlineColor     = za::Color::White,
                .outlineThickness = 1.5f,
                .radius           = world.player.drawRadius,
            });

            const bool  focus  = za::Keyboard::isKeyPressed(za::Keyboard::Key::LShift);
            const float hitR   = world.player.hitRadius;
            const auto  hitCol = focus ? za::Color{255u, 60u, 60u} : za::Color{255u, 200u, 200u};
            drawCtx.draw(za::CircleShapeData{
                .position         = world.player.pos,
                .origin           = {hitR, hitR},
                .fillColor        = hitCol,
                .outlineColor     = za::Color::White,
                .outlineThickness = 1.f,
                .radius           = hitR,
            });
        }

        {
            const float barX = 120.f;
            float       barY = 8.f;
            const float barW = worldSize.x - 240.f;
            const float barH = 10.f;

            for (const Boss& boss : world.bosses)
            {
                const float hpT = boss.maxHp > 0.f ? boss.hp / boss.maxHp : 0.f;

                drawCtx.draw(za::RectangleShapeData{
                    .position         = {barX, barY},
                    .fillColor        = {30u, 30u, 40u},
                    .outlineColor     = za::Color::White,
                    .outlineThickness = 1.f,
                    .size             = {barW, barH},
                });

                drawCtx.draw(za::RectangleShapeData{
                    .position  = {barX + 1.f, barY + 1.f},
                    .fillColor = boss.bodyTint,
                    .size      = {(barW - 2.f) * hpT, barH - 2.f},
                });

                barY += barH + 3.f;
            }
        }

        {
            const float y0 = 8.f + static_cast<float>(world.bosses.size()) * 13.f + 10.f;
            for (zb::SizeT i = 0u; i < world.log.size(); ++i)
                drawCtx.draw(font,
                             za::TextData{
                                 .position         = {10.f, y0 + static_cast<float>(i) * 16.f},
                                 .string           = world.log[i],
                                 .characterSize    = 12u,
                                 .fillColor        = za::Color::White,
                                 .outlineColor     = za::Color::Black,
                                 .outlineThickness = 1.f,
                             });
        }

        const auto drawCenteredText = [&](const zb::String& str, float y, za::Color col, unsigned size)
        {
            drawCtx.draw(font,
                         za::TextUtils::anchored(font,
                                                 za::TextData{
                                                     .position         = {worldSize.x * 0.5f, y},
                                                     .string           = str,
                                                     .characterSize    = size,
                                                     .fillColor        = col,
                                                     .outlineColor     = za::Color::Black,
                                                     .outlineThickness = 2.f,
                                                 },
                                                 {0.5f, 0.f}));
        };

        if (!world.player.alive)
        {
            drawCenteredText("GAME OVER", worldSize.y * 0.4f, {255u, 80u, 80u}, 36u);
            drawCenteredText("Press R to restart", worldSize.y * 0.4f + 48.f, za::Color::White, 18u);
        }
        else if (aliveCount == 0)
        {
            bool anyStillDying = false;
            for (const Boss& b : world.bosses)
                if (b.deathTimer > 0.f)
                    anyStillDying = true;
            if (!anyStillDying)
            {
                drawCenteredText("YOU WIN", worldSize.y * 0.4f, {120u, 255u, 140u}, 36u);
                drawCenteredText("Press R to restart", worldSize.y * 0.4f + 48.f, za::Color::White, 18u);
            }
        }

        drawCtx.draw(font,
                     za::TextData{
                         .position      = {worldSize.x - 360.f, worldSize.y - 22.f},
                         .string        = "Arrows move  Z shoot  LShift focus  R restart  F5 save  F6 load",
                         .characterSize = 11u,
                         .fillColor     = {180u, 180u, 200u},
                     });

        window.display();
    }
}
