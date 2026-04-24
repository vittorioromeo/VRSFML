////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/Scaling.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/SwapAndPop.hpp"
#include "SFML/Base/Builtin/Unreachable.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"


#if defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wc2y-extensions"
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif


namespace
{
// ============================================================================
// CORE: SFEX COROUTINE STATE + MACROS
// ============================================================================

struct SfexCoroutine
{
    int state = 0;
};


#define SFEX_CO_BEGIN                          \
    static const int _sfex_base = __COUNTER__; \
                                               \
    switch (state)                             \
    {                                          \
        case 0:;

#define SFEX_CO_YIELD(value)                    \
    do                                          \
    {                                           \
        state = (__COUNTER__ + 1) - _sfex_base; \
        return value;                           \
                                                \
        case (__COUNTER__ - _sfex_base):;       \
    } while (0)

#define SFEX_CO_RETURN(value) \
    do                        \
    {                         \
        state = 0;            \
        return value;         \
    } while (0)

#define SFEX_CO_END \
    }               \
                    \
    SFML_BASE_UNREACHABLE();

#define SFEX_CO_AWAIT(sub_call)                 \
    do                                          \
    {                                           \
        state = (__COUNTER__ + 1) - _sfex_base; \
                                                \
        case (__COUNTER__ - _sfex_base):        \
        {                                       \
            auto _r = (sub_call);               \
            if (!isFinished(_r))                \
                return _r;                      \
        }                                       \
    } while (0)

#define SFEX_CO_WAIT_UNTIL(yield_value, cond) \
    while (!(cond))                           \
    {                                         \
        SFEX_CO_YIELD(yield_value);           \
    }

#define SFEX_CO_WAIT_WHILE(yield_value, cond) \
    while (cond)                              \
    {                                         \
        SFEX_CO_YIELD(yield_value);           \
    }


// ============================================================================
// PARALLEL COMPOSITION
// ============================================================================

template <typename Time = float>
struct SfexParallelMixin
{
    Time parallelWait{};
    bool parallelDone = false;
};

template <typename Child, typename Time, typename... CallArgs>
[[nodiscard]] bool tickInParallel(Child& child, Time dt, CallArgs&&... args)
{
    if (child.parallelDone)
        return false;

    if (child.parallelWait > Time{})
    {
        child.parallelWait -= dt;
        if (child.parallelWait < Time{})
            child.parallelWait = Time{};
        return true;
    }

    yieldApply(child(static_cast<CallArgs&&>(args)...), child);
    return !child.parallelDone;
}

template <typename Time, typename Ctx, typename... Children>
[[nodiscard]] bool tickAllInParallel(Time dt, Ctx& ctx, Children&... children)
{
    for (const bool r : {tickInParallel(children, dt, ctx)...})
        if (r)
            return false;

    return true;
}

template <typename Time, typename Ctx, typename... Children>
[[nodiscard]] bool tickAnyInParallel(Time dt, Ctx& ctx, Children&... children)
{
    for (const bool r : {tickInParallel(children, dt, ctx)...})
        if (!r)
            return true;

    return false;
}

#define SFEX_CO_AWAIT_ALL(dt_expr, ctx_expr, next_yield, ...)  \
    while (!tickAllInParallel(dt_expr, ctx_expr, __VA_ARGS__)) \
    {                                                          \
        SFEX_CO_YIELD(next_yield);                             \
    }

#define SFEX_CO_AWAIT_ANY(dt_expr, ctx_expr, next_yield, ...)  \
    while (!tickAnyInParallel(dt_expr, ctx_expr, __VA_ARGS__)) \
    {                                                          \
        SFEX_CO_YIELD(next_yield);                             \
    }


// ============================================================================
// YIELD TYPE
// ============================================================================

struct NextFrame
{
};

struct Wait
{
    float seconds;
};

struct Done
{
};

struct Yield : sf::base::Variant<NextFrame, Wait, Done>
{
    using Base = sf::base::Variant<NextFrame, Wait, Done>;

    explicit(false) Yield(const auto& x) noexcept : Base{x}
    {
    }
};

[[nodiscard]] inline bool isFinished(const Yield& y) noexcept
{
    return y.is<Done>();
}


// ============================================================================
// LERP + TWEEN
// ============================================================================

[[nodiscard, gnu::always_inline]] inline constexpr sf::Vec2f lerp(sf::Vec2f a, sf::Vec2f b, float t) noexcept
{
    return a + (b - a) * t;
}

[[nodiscard, gnu::always_inline]] inline constexpr float lerp(float a, float b, float t) noexcept
{
    return a + (b - a) * t;
}


template <typename T>
struct Tween
{
    float t = 0.f;
    T     from{};
    T     to{};

    void start(T fromValue, T toValue)
    {
        t    = 0.f;
        from = fromValue;
        to   = toValue;
    }

    bool step(float dt, float duration)
    {
        if (t >= 1.f)
            return false;

        t = sf::base::min(t + dt / duration, 1.f);
        return true;
    }

    template <typename Easing>
    [[nodiscard]] T sample(Easing&& easing) const
    {
        return lerp(from, to, easing(t));
    }
};


// ============================================================================
// GAME ENTITIES
// ============================================================================

constexpr sf::Vec2f worldSize{800.f, 600.f};

struct Bullet
{
    sf::Vec2f pos;
    sf::Vec2f vel;
    float     radius = 6.f;
    sf::Color color  = sf::Color::White;
};

struct PlayerBullet
{
    sf::Vec2f pos;
    sf::Vec2f vel;
    float     radius = 4.f;
    bool      dead   = false;
};

struct Player
{
    sf::Vec2f pos           = {worldSize.x * 0.5f, worldSize.y - 60.f};
    float     drawRadius    = 9.f;
    float     hitRadius     = 3.f;
    bool      alive         = true;
    float     shootCooldown = 0.f;
};

struct Boss
{
    sf::Vec2f homePos;
    sf::Vec2f pos;
    sf::Vec2f vel        = {0.f, 0.f};
    float     radius     = 30.f;
    float     scale      = 1.f;
    sf::Color color      = {120u, 170u, 255u};
    sf::Color bodyTint   = {120u, 170u, 255u};
    bool      weakPoint  = false;
    float     maxHp      = 120.f;
    float     hp         = 120.f;
    bool      alive      = true;
    float     deathTimer = 0.f;

    // Per-boss coroutine scheduling.
    float waitTimer = 0.f;
    float waitCarry = 0.f;
    float restartIn = 0.f;
};


// ============================================================================
// WORLD
//
// All the shared game state lives here. Note: the per-boss *scripts*
// (`BossFight` instances) are NOT stored in `World` -- they live next to
// `World` (in `main` and in `Snapshot`) as a parallel vector. Doing it
// this way keeps `World` complete *before* the sub-coroutines are defined,
// which in turn lets those sub-coroutines have plain (non-template)
// `operator()(BossCtx)` methods instead of templated ones.
// ============================================================================

struct World
{
    float dt   = 0.f;
    float time = 0.f;

    Player player{};

    sf::base::Vector<Boss>             bosses;
    sf::base::Vector<Bullet>           bullets;
    sf::base::Vector<PlayerBullet>     playerBullets;
    sf::base::Vector<sf::base::String> log;

    static constexpr sf::base::SizeT maxLogLines = 22u;

    void addLog(const char* who, const sf::base::String& what)
    {
        sf::base::String line = "[t=";
        sf::base::appendToString(line, time);
        line += "s] ";
        line += who;
        line += "  ";
        line += what;

        log.emplaceBack(SFML_BASE_MOVE(line));

        if (log.size() > maxLogLines)
            log.erase(log.begin());
    }

    void spawnBulletRing(sf::Vec2f source, int count, float speed, sf::Angle startAngle, sf::Color color, float bulletRadius)
    {
        const auto total = sf::radians(sf::base::tau);
        for (int i = 0; i < count; ++i)
        {
            const sf::Angle a = startAngle + total * (static_cast<float>(i) / static_cast<float>(count));
            bullets.emplaceBack(Bullet{
                .pos    = source,
                .vel    = sf::Vec2f::fromAngle(speed, a),
                .radius = bulletRadius,
                .color  = color,
            });
        }
    }

    void spawnAimedBullet(sf::Vec2f source, sf::Vec2f target, float speed, sf::Color color)
    {
        sf::Vec2f   dir = target - source;
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

    // Populate `bosses` with the fixed starting roster. The caller is
    // responsible for resizing any parallel `bossScripts` vector to match.
    void initBosses()
    {
        bosses.clear();

        struct Spawn
        {
            sf::Vec2f home{};
            sf::Color tint{};
            float     maxHp = 0.f;
        };

        const Spawn spawns[]{
            {.home = {worldSize.x * 0.25f, 140.f}, .tint = {120u, 170u, 255u}, .maxHp = 100.f},
            {.home = {worldSize.x * 0.50f, 100.f}, .tint = {255u, 180u, 120u}, .maxHp = 140.f},
            {.home = {worldSize.x * 0.75f, 140.f}, .tint = {200u, 120u, 255u}, .maxHp = 100.f},
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

        addLog("SYSTEM", "spawned " + sf::base::toString(bosses.size()) + " bosses");
    }
};


// Context threaded through every coroutine.
struct BossCtx
{
    World& world;
    Boss&  self;
};


// ============================================================================
// Game-layer glue
// ============================================================================

template <typename Child>
void yieldApply(const Yield& y, Child& child)
{
    y.recursiveMatch( //
        [&](NextFrame) {},
        [&](const Wait& w) { child.parallelWait = w.seconds; },
        [&](Done) { child.parallelDone = true; });
}

struct BossCoroutine : SfexCoroutine, SfexParallelMixin<float>
{
};

#define BOSS_CO_AWAIT_ALL(...)   SFEX_CO_AWAIT_ALL(ctx.world.dt, ctx, NextFrame{}, __VA_ARGS__)
#define BOSS_CO_AWAIT_ANY(...)   SFEX_CO_AWAIT_ANY(ctx.world.dt, ctx, NextFrame{}, __VA_ARGS__)
#define BOSS_CO_WAIT_UNTIL(cond) SFEX_CO_WAIT_UNTIL(NextFrame{}, cond)
#define BOSS_CO_WAIT_WHILE(cond) SFEX_CO_WAIT_WHILE(NextFrame{}, cond)


// ============================================================================
// SUB-COROUTINES
// ============================================================================

struct BulletRingBarrage : BossCoroutine
{
    int       ringsToFire    = 3;
    int       bulletsPerRing = 12;
    float     gapSeconds     = 0.30f;
    float     bulletSpeed    = 140.f;
    float     bulletRadius   = 6.f;
    sf::Angle ringAngleStep  = sf::degrees(7.f);
    sf::Color bulletColor    = {255u, 180u, 80u};

    int       i         = 0;
    sf::Angle ringAngle = sf::Angle::Zero;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        ringAngle = sf::Angle::Zero;
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


struct DashAttack : BossCoroutine
{
    sf::Vec2f dashTarget;
    float     dashTime = 0.25f;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        ctx.self.color = {255u, 80u, 80u};
        SFEX_CO_YIELD(Wait{0.50f});

        ctx.self.color = {255u, 220u, 60u};
        SFEX_CO_YIELD(Wait{0.15f});

        ctx.self.vel = (dashTarget - ctx.self.pos) / dashTime;
        for (int k = -2; k <= 2; ++k)
        {
            const sf::Angle a = ctx.self.vel.angle() + sf::degrees(static_cast<float>(k) * 10.f);
            ctx.world.bullets.emplaceBack(Bullet{
                .pos    = ctx.self.pos,
                .vel    = sf::Vec2f::fromAngle(220.f, a),
                .radius = 6.f,
                .color  = {255u, 140u, 40u},
            });
        }
        SFEX_CO_YIELD(Wait{dashTime});

        ctx.self.vel   = {0.f, 0.f};
        ctx.self.color = ctx.self.bodyTint;
        SFEX_CO_YIELD(Wait{0.40f});

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


struct VulnerabilityWindow : BossCoroutine
{
    float duration = 1.0f;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        ctx.self.weakPoint = true;
        ctx.self.color     = {130u, 230u, 150u};
        SFEX_CO_YIELD(Wait{duration});

        ctx.self.weakPoint = false;
        ctx.self.color     = ctx.self.bodyTint;
        SFEX_CO_YIELD(Wait{0.25f});

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


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


struct SlamAttack : BossCoroutine
{
    sf::Vec2f windupOffset = {0.f, -80.f};
    sf::Vec2f slamOffset   = {0.f, 180.f};

    Tween<sf::Vec2f> tween{};

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

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

        ctx.world.spawnBulletRing(ctx.self.pos, 18, 240.f, sf::Angle::Zero, {255u, 140u, 40u}, 6.f);
        SFEX_CO_YIELD(Wait{0.15f});

        ctx.self.color = ctx.self.bodyTint;
        tween.start(ctx.self.pos, ctx.self.homePos);
        while (tween.step(ctx.world.dt, 0.50f))
        {
            ctx.self.pos = tween.sample(easeInOutSine);
            SFEX_CO_YIELD(NextFrame{});
        }

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


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
                                      sf::radians(static_cast<float>(p) * 0.13f),
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


struct SweepAttack : BossCoroutine
{
    float sweepRange    = 120.f;
    float sweepDuration = 1.60f;
    float spawnEvery    = 0.10f;

    Tween<sf::Vec2f> tween{};
    float            spawnTimer = 0.f;

    Yield operator()(BossCtx ctx)
    {
        SFEX_CO_BEGIN;

        ctx.self.color = {180u, 140u, 255u};
        tween.start(ctx.self.pos, ctx.self.homePos + sf::Vec2f{-sweepRange, 0.f});
        while (tween.step(ctx.world.dt, sweepDuration))
        {
            ctx.self.pos = tween.sample(easeInOutQuint);
            SFEX_CO_YIELD(NextFrame{});
        }

        tween.start(ctx.self.pos, ctx.self.homePos + sf::Vec2f{sweepRange, 0.f});
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

        ctx.self.color = ctx.self.bodyTint;
        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


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


// ============================================================================
// TOP-LEVEL BOSS FIGHT
// ============================================================================

struct BossFight : BossCoroutine
{
    BulletRingBarrage   barrage;
    DashAttack          dash;
    VulnerabilityWindow window;
    AimedVolley         volley;
    SlamAttack          slam;
    PulseAttack         pulse;
    SweepAttack         sweep;
    Timer               timer;
    BulletRingBarrage   parallelBarrage;
    SweepAttack         parallelSweep;
    AimedVolley         parallelVolley;

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
            .dashTarget = ctx.self.homePos + sf::Vec2f{-80.f, 40.f},
        };
        SFEX_CO_AWAIT(dash(ctx));

        dash = DashAttack{
            .dashTarget = ctx.self.homePos + sf::Vec2f{80.f, 40.f},
        };
        SFEX_CO_AWAIT(dash(ctx));

        window = VulnerabilityWindow{
            .duration = 0.80f,
        };
        SFEX_CO_AWAIT(window(ctx));

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
            .ringAngleStep  = sf::degrees(11.f),
            .bulletColor    = {255u, 120u, 120u},
        };
        SFEX_CO_AWAIT(barrage(ctx));

        SFEX_CO_RETURN(Done{});
        SFEX_CO_END;
    }
};


// ============================================================================
// SNAPSHOT
//
// Scripts are stored alongside `World` (not inside it), so the snapshot
// captures both pieces explicitly. Remains flat and copyable.
// ============================================================================

struct Snapshot
{
    static constexpr sf::base::U32 currentVersion = 1u;

    World                       world;
    sf::base::Vector<BossFight> bossScripts;
    sf::base::U32               version = currentVersion;
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
int main()
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = worldSize.toVec2u(),
                          .title     = "SFML Coroutine (macro-based boss fight)",
                          .resizable = true,
                          .vsync     = true,
                      })
                      .value();

    auto windowView = window.computeView();

    const auto font = sf::Font::openFromFile("resources/tuffy.ttf").value();

    World                       world;
    sf::base::Vector<BossFight> bossScripts; // parallel to `world.bosses`

    const auto resetScriptsToBosses = [&] { bossScripts.resize(world.bosses.size()); };

    world.initBosses();
    resetScriptsToBosses();

    sf::Clock                    frameClock;
    sf::base::Optional<Snapshot> quickSave;

    const auto doRestart = [&]
    {
        world = World{};
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
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, worldSize, windowView))
                continue;

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
            {
                if (kp->code == sf::Keyboard::Key::R)
                    requestRestart = true;
                else if (kp->code == sf::Keyboard::Key::F5)
                    requestQuickSave = true;
                else if (kp->code == sf::Keyboard::Key::F6)
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

        sf::base::SizeT aliveCount = 0;
        for (const Boss& b : world.bosses)
            if (b.alive)
                ++aliveCount;

        const bool gameActive = world.player.alive && aliveCount > 0;

        // ---- Player input ----
        if (gameActive)
        {
            using K           = sf::Keyboard::Key;
            const bool  focus = sf::Keyboard::isKeyPressed(K::LShift);
            const float speed = focus ? 140.f : 320.f;

            sf::Vec2f move{0.f, 0.f};
            if (sf::Keyboard::isKeyPressed(K::Left))
                move.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(K::Right))
                move.x += 1.f;
            if (sf::Keyboard::isKeyPressed(K::Up))
                move.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(K::Down))
                move.y += 1.f;

            if (move.lengthSquared() > 0.f)
            {
                const float invLen = 1.f / move.length();
                move *= invLen;
            }
            world.player.pos += move * (speed * dt);

            const float m      = world.player.drawRadius;
            world.player.pos.x = sf::base::clamp(world.player.pos.x, m, worldSize.x - m);
            world.player.pos.y = sf::base::clamp(world.player.pos.y, m, worldSize.y - m);

            world.player.shootCooldown = sf::base::max(world.player.shootCooldown - dt, 0.f);
            if (sf::Keyboard::isKeyPressed(K::Z) && world.player.shootCooldown == 0.f)
            {
                world.player.shootCooldown = 0.07f;
                world.playerBullets.emplaceBack(PlayerBullet{
                    .pos = world.player.pos + sf::Vec2f{-9.f, -6.f},
                    .vel = {0.f, -900.f},
                });
                world.playerBullets.emplaceBack(PlayerBullet{
                    .pos = world.player.pos + sf::Vec2f{9.f, -6.f},
                    .vel = {0.f, -900.f},
                });
            }
        }

        // ---- Per-boss coroutine driver ----
        if (gameActive)
        {
            for (sf::base::SizeT i = 0u; i < world.bosses.size(); ++i)
            {
                Boss& boss = world.bosses[i];
                if (!boss.alive)
                    continue;

                BossCtx    ctx{world, boss};
                BossFight& script = bossScripts[i];

                if (boss.restartIn > 0.f)
                {
                    boss.restartIn = sf::base::max(boss.restartIn - dt, 0.f);
                    if (boss.restartIn == 0.f)
                    {
                        script         = BossFight{};
                        boss.waitCarry = 0.f;
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
                boss.weakPoint  = false;
                world.addLog("BOSS", "defeated!");
            }
            if (!boss.alive && boss.deathTimer > 0.f)
            {
                boss.deathTimer = sf::base::max(boss.deathTimer - dt, 0.f);
                const float k   = boss.deathTimer / 1.5f;
                boss.scale      = k;
                boss.color.a    = static_cast<sf::base::U8>(255.f * k);
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
        sf::base::vectorSwapAndPopIf(world.bullets,
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
        sf::base::vectorSwapAndPopIf(world.playerBullets,
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
                        boss.hp = sf::base::max(boss.hp - 1.f, 0.f);
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
            drawCtx.draw(sf::CircleShapeData{
                .position         = boss.pos,
                .origin           = {bossDrawRadius, bossDrawRadius},
                .fillColor        = boss.color,
                .outlineColor     = sf::Color::White,
                .outlineThickness = 2.f,
                .radius           = bossDrawRadius,
            });

            if (boss.weakPoint)
            {
                const float wpRadius = boss.radius * 0.45f;
                drawCtx.draw(sf::CircleShapeData{
                    .position         = boss.pos,
                    .origin           = {wpRadius, wpRadius},
                    .fillColor        = {255u, 255u, 120u},
                    .outlineColor     = sf::Color::Red,
                    .outlineThickness = 2.f,
                    .radius           = wpRadius,
                });
            }
        }

        for (const Bullet& b : world.bullets)
            drawCtx.draw(sf::CircleShapeData{
                .position         = b.pos,
                .origin           = {b.radius, b.radius},
                .fillColor        = b.color,
                .outlineColor     = {0u, 0u, 0u, 180u},
                .outlineThickness = 1.f,
                .radius           = b.radius,
            });

        for (const PlayerBullet& b : world.playerBullets)
            drawCtx.draw(sf::RectangleShapeData{
                .position         = b.pos,
                .origin           = {3.f, 7.f},
                .fillColor        = {180u, 240u, 255u},
                .outlineColor     = {40u, 120u, 200u},
                .outlineThickness = 1.f,
                .size             = {6.f, 14.f},
            });

        if (world.player.alive)
        {
            drawCtx.draw(sf::CircleShapeData{
                .position         = world.player.pos,
                .origin           = {world.player.drawRadius, world.player.drawRadius},
                .fillColor        = {120u, 220u, 255u},
                .outlineColor     = sf::Color::White,
                .outlineThickness = 1.5f,
                .radius           = world.player.drawRadius,
            });

            const bool  focus  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
            const float hitR   = world.player.hitRadius;
            const auto  hitCol = focus ? sf::Color{255u, 60u, 60u} : sf::Color{255u, 200u, 200u};
            drawCtx.draw(sf::CircleShapeData{
                .position         = world.player.pos,
                .origin           = {hitR, hitR},
                .fillColor        = hitCol,
                .outlineColor     = sf::Color::White,
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

                drawCtx.draw(sf::RectangleShapeData{
                    .position         = {barX, barY},
                    .fillColor        = {30u, 30u, 40u},
                    .outlineColor     = sf::Color::White,
                    .outlineThickness = 1.f,
                    .size             = {barW, barH},
                });
                drawCtx.draw(sf::RectangleShapeData{
                    .position  = {barX + 1.f, barY + 1.f},
                    .fillColor = boss.bodyTint,
                    .size      = {(barW - 2.f) * hpT, barH - 2.f},
                });

                barY += barH + 3.f;
            }
        }

        {
            const float y0 = 8.f + static_cast<float>(world.bosses.size()) * 13.f + 10.f;
            for (sf::base::SizeT i = 0u; i < world.log.size(); ++i)
                drawCtx.draw(font,
                             sf::TextData{
                                 .position         = {10.f, y0 + static_cast<float>(i) * 16.f},
                                 .string           = world.log[i],
                                 .characterSize    = 12u,
                                 .fillColor        = sf::Color::White,
                                 .outlineColor     = sf::Color::Black,
                                 .outlineThickness = 1.f,
                             });
        }

        const auto drawCenteredText = [&](const sf::base::String& str, float y, sf::Color col, unsigned size)
        {
            drawCtx.draw(font,
                         sf::TextData{
                             .position      = {worldSize.x * 0.5f - static_cast<float>(str.size()) * 0.25f * size, y},
                             .string        = str,
                             .characterSize = size,
                             .fillColor     = col,
                             .outlineColor  = sf::Color::Black,
                             .outlineThickness = 2.f,
                         });
        };

        if (!world.player.alive)
        {
            drawCenteredText("YOU DIED", worldSize.y * 0.4f, {255u, 80u, 80u}, 36u);
            drawCenteredText("Press R to restart", worldSize.y * 0.4f + 48.f, sf::Color::White, 18u);
        }
        else if (aliveCount == 0)
        {
            bool anyStillDying = false;
            for (const Boss& b : world.bosses)
                if (b.deathTimer > 0.f)
                    anyStillDying = true;
            if (!anyStillDying)
            {
                drawCenteredText("BOSSES DEFEATED", worldSize.y * 0.4f, {120u, 255u, 140u}, 36u);
                drawCenteredText("Press R to restart", worldSize.y * 0.4f + 48.f, sf::Color::White, 18u);
            }
        }

        drawCtx.draw(font,
                     sf::TextData{
                         .position      = {worldSize.x - 360.f, worldSize.y - 22.f},
                         .string        = "Arrows move  Z shoot  LShift focus  R restart  F5 save  F6 load",
                         .characterSize = 11u,
                         .fillColor     = {180u, 180u, 200u},
                     });

        window.display();
    }
}
