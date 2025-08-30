#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/SoA.hpp"     // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/View.hpp" // used

#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <cstdio>


namespace
{
////////////////////////////////////////////////////////////
RNGFast rng;


////////////////////////////////////////////////////////////
sf::Texture*  txAtlas;
sf::FloatRect txrSmoke;
sf::FloatRect txrFire;
sf::FloatRect txrRocket;


////////////////////////////////////////////////////////////
void drawParticleImpl(sf::RenderTarget&   rt,
                      const sf::Vec2f     position,
                      const sf::Vec2f     scale,
                      const float         rotation,
                      const sf::FloatRect txr,
                      const float         opacity)
{
    rt.draw(
        sf::Sprite{
            .position    = position,
            .scale       = scale,
            .origin      = txr.size / 2.f,
            .rotation    = sf::radians(rotation),
            .textureRect = txr,
            .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(opacity * 255.f)),
        },
        sf::RenderStates{.texture = txAtlas});
}


////////////////////////////////////////////////////////////
namespace OOP
{
////////////////////////////////////////////////////////////
struct World;

////////////////////////////////////////////////////////////
struct Entity
{
    World* world = nullptr;

    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    bool alive = true;

    virtual ~Entity() = default;

    virtual void update(float dt)
    {
        position += velocity * dt;
        velocity += acceleration * dt;
    }

    virtual void draw(sf::RenderTarget&) = 0;
};

////////////////////////////////////////////////////////////
struct World
{
    sf::base::Vector<sf::base::UniquePtr<Entity>> entities;

    void cleanup()
    {
        sf::base::vectorSwapAndPopIf(entities, [](const auto& entity) { return !entity->alive; });
    }

    void update(float dt)
    {
        for (sf::base::SizeT i = 0; i < entities.size(); ++i)
            entities[i]->update(dt);
    }

    void draw(sf::RenderTarget& rt)
    {
        for (const auto& entity : entities)
            entity->draw(rt);
    }

    template <typename T>
    T& addEntity()
    {
        auto  newEntity = sf::base::makeUnique<T>();
        auto& result    = *newEntity;

        entities.emplaceBack(SFML_BASE_MOVE(newEntity));

        result.world = this;
        return result;
    }
};

////////////////////////////////////////////////////////////
struct Emitter : Entity
{
    float lifetime;
    float lifetimeDecayRate;

    float spawnTimer;
    float spawnRate;

    virtual void spawnParticle() = 0;

    void update(float dt) override
    {
        Entity::update(dt);

        lifetime += lifetimeDecayRate * dt;
        spawnTimer += spawnRate * dt;

        for (; spawnTimer >= 1.f; spawnTimer -= 1.f)
            spawnParticle();

        alive = lifetime > 0.f;
    }

    void draw(sf::RenderTarget&) override
    {
    }
};

////////////////////////////////////////////////////////////
struct Particle : Entity
{
    float lifetime;
    float scale;
    float opacity;
    float rotation;

    float lifetimeDecayRate;
    float scaleRate;
    float opacityChange;
    float angularVelocity;

    void update(float dt) override
    {
        Entity::update(dt);

        lifetime += lifetimeDecayRate * dt;
        scale += scaleRate * dt;
        opacity += opacityChange * dt;
        rotation += angularVelocity * dt;

        alive = lifetime > 0.f;

        if (scale <= 0.f)
            scale = 0.f;

        if (opacity <= 0.f)
            opacity = 0.f;
    }
};

////////////////////////////////////////////////////////////
struct SmokeParticle : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        drawParticleImpl(rt, position, {scale, scale}, rotation, txrSmoke, opacity);
    }
};

////////////////////////////////////////////////////////////
struct FireParticle : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        drawParticleImpl(rt, position, {scale, scale}, rotation, txrFire, opacity);
    }
};

////////////////////////////////////////////////////////////
struct SmokeEmitter : Emitter
{
    void spawnParticle() override
    {
        auto& p = world->addEntity<SmokeParticle>();

        p.position     = position;
        p.velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f;
        p.acceleration = {0.f, -0.011f};

        p.lifetime = 1.f;
        p.scale    = rng.getF(0.0025f, 0.0035f);
        p.opacity  = rng.getF(0.05f, 0.25f);
        p.rotation = rng.getF(0.f, 6.28f);

        p.lifetimeDecayRate = -0.005f;
        p.scaleRate         = rng.getF(0.001f, 0.003f) * 2.75f;
        p.opacityChange     = -rng.getF(0.001f, 0.002f) * 3.25f;
        p.angularVelocity   = rng.getF(-0.02f, 0.02f);
    }
};

////////////////////////////////////////////////////////////
struct FireEmitter : Emitter
{
    void spawnParticle() override
    {
        auto& p = world->addEntity<FireParticle>();

        p.position     = position;
        p.velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f});
        p.acceleration = {0.f, 0.07f};

        p.lifetime = 1.f;
        p.scale    = rng.getF(0.5f, 0.7f) * 0.085f;
        p.opacity  = rng.getF(0.2f, 0.4f) * 0.85f;
        p.rotation = 0.f;

        p.lifetimeDecayRate = -0.005f;
        p.scaleRate         = -rng.getF(0.001f, 0.003f) * 0.25f;
        p.opacityChange     = -0.001f;
        p.angularVelocity   = rng.getF(-0.002f, 0.002f);
    }
};

////////////////////////////////////////////////////////////
struct Rocket : Entity
{
    SmokeEmitter* smokeEmitter = nullptr;
    FireEmitter*  fireEmitter  = nullptr;

    bool init = false;

    void update(float dt) override
    {
        Entity::update(dt);

        if (!init)
        {
            init = true;

            auto& se             = world->addEntity<SmokeEmitter>();
            se.position          = position;
            se.velocity          = {};
            se.acceleration      = {};
            se.lifetime          = 100.f;
            se.lifetimeDecayRate = -0.01f;
            se.spawnTimer        = 0.f;
            se.spawnRate         = 2.5f;
            smokeEmitter         = &se;

            auto& fe             = world->addEntity<FireEmitter>();
            fe.position          = position;
            fe.velocity          = {};
            fe.acceleration      = {};
            fe.lifetime          = 100.f;
            fe.lifetimeDecayRate = -0.01f;
            fe.spawnTimer        = 0.f;
            fe.spawnRate         = 1.25f;
            fireEmitter          = &fe;
        }

        velocity += acceleration * dt;
        position += velocity * dt;

        velocity = velocity.clampMaxLength(2.5f);

        smokeEmitter->position = position - sf::Vec2f{12.f, 0.f};
        fireEmitter->position  = position - sf::Vec2f{12.f, 0.f};

        if (position.x > 1680.f + 100.f)
        {
            alive = false;

            smokeEmitter->lifetime = 0.f;
            fireEmitter->lifetime  = 0.f;
        }
    }

    void draw(sf::RenderTarget& rt) override
    {
        rt.draw(sf::Sprite{.position    = position,
                           .scale       = {0.15f, 0.15f},
                           .origin      = txrRocket.size / 2.f,
                           .rotation    = sf::radians(0.f),
                           .textureRect = txrRocket,
                           .color       = sf::Color::White},
                sf::RenderStates{.texture = txAtlas});
    }
};

} // namespace OOP


////////////////////////////////////////////////////////////
namespace AOS
{
////////////////////////////////////////////////////////////
enum class ParticleType
{
    Smoke,
    Fire
};

////////////////////////////////////////////////////////////
struct Emitter
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float lifetime;
    float lifetimeDecayRate;

    float spawnTimer;
    float spawnRate;

    ParticleType type;
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float lifetime;
    float scale;
    float opacity;
    float rotation;

    float lifetimeDecayRate;
    float scaleRate;
    float opacityChange;
    float angularVelocity;

    ParticleType type;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::SizeT smokeEmitterIdx;
    sf::base::SizeT fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World
{
    sf::base::Vector<sf::base::Optional<Emitter>> emitters;
    sf::base::Vector<Particle>                    particles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT addEmitter(const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0; i < emitters.size(); ++i)
        {
            if (!emitters[i].hasValue())
            {
                emitters[i].emplace(emitter);
                return i;
            }
        }

        emitters.emplaceBack(emitter);
        return emitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    Particle& addParticle(const Particle& particle)
    {
        particles.emplaceBack(particle);
        return particles.back();
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        auto& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 2.5f,
            .type              = ParticleType::Smoke,
        });

        rocket.fireEmitterIdx = addEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 1.25f,
            .type              = ParticleType::Fire,
        });

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        for (Particle& p : particles)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;

            p.lifetime += p.lifetimeDecayRate * dt;
            p.scale += p.scaleRate * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.angularVelocity * dt;

            if (p.scale <= 0.f)
                p.scale = 0.f;

            if (p.opacity <= 0.f)
                p.opacity = 0.f;
        }

        for (auto& e : emitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->lifetime += e->lifetimeDecayRate * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
            {
                if (e->type == ParticleType::Smoke)
                {
                    addParticle(
                        {.position     = e->position,
                         .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                         .acceleration = {0.f, -0.011f},

                         .lifetime = 1.f,
                         .scale    = rng.getF(0.0025f, 0.0035f),
                         .opacity  = rng.getF(0.05f, 0.25f),
                         .rotation = rng.getF(0.f, 6.28f),

                         .lifetimeDecayRate = -0.005f,
                         .scaleRate         = rng.getF(0.001f, 0.003f) * 2.75f,
                         .opacityChange     = -rng.getF(0.001f, 0.002f) * 3.25f,
                         .angularVelocity   = rng.getF(-0.02f, 0.02f),

                         .type = ParticleType::Smoke});
                }
                else if (e->type == ParticleType::Fire)
                {
                    addParticle({
                        .position     = e->position,
                        .velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                        .acceleration = {0.f, 0.07f},

                        .lifetime = 1.f,
                        .scale    = rng.getF(0.5f, 0.7f) * 0.085f,
                        .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                        .rotation = rng.getF(0.f, 6.28f),

                        .lifetimeDecayRate = -0.005f,
                        .scaleRate         = -rng.getF(0.001f, 0.003f) * 0.25f,
                        .opacityChange     = -0.001f,
                        .angularVelocity   = rng.getF(-0.002f, 0.002f),

                        .type = ParticleType::Fire,
                    });
                }
            }
        }

        for (auto& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (auto& se = emitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (auto& fe = emitters[r.fireEmitterIdx])
                fe->position = r.position + sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        for (auto& e : emitters)
            if (e.hasValue() && e->lifetime <= 0.f)
                e.reset();

        sf::base::vectorSwapAndPopIf(particles, [](const auto& p) { return p.lifetime <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const auto& r)
        {
            const bool outOfBounds = r.position.x > 1680.f + 100.f;

            if (outOfBounds)
            {
                auto& se = emitters[r.smokeEmitterIdx];
                if (se.hasValue())
                    se.reset();

                auto& fe = emitters[r.fireEmitterIdx];
                if (fe.hasValue())
                    fe.reset();
            }

            return outOfBounds;
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        for (const auto& p : particles)
        {
            auto* txr = (p.type == ParticleType::Smoke) ? &txrSmoke : &txrFire;
            drawParticleImpl(rt, p.position, {p.scale, p.scale}, p.rotation, *txr, p.opacity);
        }

        for (const auto& r : rockets)
        {
            auto* txr = &txrRocket;

            rt.draw(
                sf::Sprite{
                    .position    = r.position,
                    .scale       = {0.15f, 0.15f},
                    .origin      = txr->size / 2.f,
                    .rotation    = sf::radians(0.f),
                    .textureRect = *txr,
                    .color       = sf::Color::White,
                },
                sf::RenderStates{.texture = txAtlas});
        }
    }
};

} // namespace AOS


////////////////////////////////////////////////////////////
namespace AOSImproved
{
////////////////////////////////////////////////////////////
struct Emitter
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float lifetime;
    float lifetimeDecayRate;

    float spawnTimer;
    float spawnRate;
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float lifetime;
    float scale;
    float opacity;
    float rotation;

    float lifetimeDecayRate;
    float scaleRate;
    float opacityChange;
    float angularVelocity;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::SizeT smokeEmitterIdx;
    sf::base::SizeT fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World
{
    sf::base::Vector<sf::base::Optional<Emitter>> smokeEmitters;
    sf::base::Vector<sf::base::Optional<Emitter>> fireEmitters;
    sf::base::Vector<Particle>                    smokeParticles;
    sf::base::Vector<Particle>                    fireParticles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT addSmokeEmitter(const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0; i < smokeEmitters.size(); ++i)
        {
            if (!smokeEmitters[i].hasValue())
            {
                smokeEmitters[i].emplace(emitter);
                return i;
            }
        }

        smokeEmitters.emplaceBack(emitter);
        return smokeEmitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT addFireEmitter(const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0; i < fireEmitters.size(); ++i)
        {
            if (!fireEmitters[i].hasValue())
            {
                fireEmitters[i].emplace(emitter);
                return i;
            }
        }

        fireEmitters.emplaceBack(emitter);
        return fireEmitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    Particle& addSmokeParticle(const Particle& particle)
    {
        return smokeParticles.emplaceBack(particle);
    }

    ////////////////////////////////////////////////////////////
    Particle& addFireParticle(const Particle& particle)
    {
        return fireParticles.emplaceBack(particle);
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        auto& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addSmokeEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 2.5f,
        });

        rocket.fireEmitterIdx = addFireEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 1.25f,
        });

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        auto updateParticle = [&](Particle& p)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;

            p.lifetime += p.lifetimeDecayRate * dt;
            p.scale += p.scaleRate * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.angularVelocity * dt;

            if (p.scale <= 0.f)
                p.scale = 0.f;

            if (p.opacity <= 0.f)
                p.opacity = 0.f;
        };

        for (auto& p : smokeParticles)
            updateParticle(p);

        for (auto& p : fireParticles)
            updateParticle(p);

        for (auto& e : smokeEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->lifetime += e->lifetimeDecayRate * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                addSmokeParticle({
                    .position     = e->position,
                    .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                    .acceleration = {0.f, -0.011f},

                    .lifetime = 1.f,
                    .scale    = rng.getF(0.0025f, 0.0035f),
                    .opacity  = rng.getF(0.05f, 0.25f),
                    .rotation = rng.getF(0.f, 6.28f),

                    .lifetimeDecayRate = -0.005f,
                    .scaleRate         = rng.getF(0.001f, 0.003f) * 2.75f,
                    .opacityChange     = -rng.getF(0.001f, 0.002f) * 3.25f,
                    .angularVelocity   = rng.getF(-0.02f, 0.02f),
                });
        }

        for (auto& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->lifetime += e->lifetimeDecayRate * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                addFireParticle({
                    .position     = e->position,
                    .velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                    .acceleration = {0.f, 0.07f},

                    .lifetime = 1.f,
                    .scale    = rng.getF(0.5f, 0.7f) * 0.085f,
                    .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                    .rotation = rng.getF(0.f, 6.28f),

                    .lifetimeDecayRate = -0.005f,
                    .scaleRate         = -rng.getF(0.001f, 0.003f) * 0.25f,
                    .opacityChange     = -0.001f,
                    .angularVelocity   = rng.getF(-0.002f, 0.002f),
                });
        }

        for (Rocket& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (auto& se = smokeEmitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (auto& fe = fireEmitters[r.fireEmitterIdx])
                fe->position = r.position + sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        for (auto& e : smokeEmitters)
            if (e.hasValue() && e->lifetime <= 0.f)
                e.reset();

        for (auto& e : fireEmitters)
            if (e.hasValue() && e->lifetime <= 0.f)
                e.reset();

        sf::base::vectorSwapAndPopIf(smokeParticles, [](const auto& p) { return p.lifetime <= 0.f; });
        sf::base::vectorSwapAndPopIf(fireParticles, [](const auto& p) { return p.lifetime <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const auto& r)
        {
            const bool outOfBounds = r.position.x > 1680.f + 100.f;

            if (outOfBounds)
            {
                auto& se = smokeEmitters[r.smokeEmitterIdx];
                if (se.hasValue())
                    se.reset();

                auto& fe = fireEmitters[r.fireEmitterIdx];
                if (fe.hasValue())
                    fe.reset();
            }

            return outOfBounds;
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        for (const auto& p : smokeParticles)
            drawParticleImpl(rt, p.position, {p.scale, p.scale}, p.rotation, txrSmoke, p.opacity);

        for (const auto& p : fireParticles)
            drawParticleImpl(rt, p.position, {p.scale, p.scale}, p.rotation, txrFire, p.opacity);

        for (const auto& r : rockets)
        {
            auto* txr = &txrRocket;

            rt.draw(
                sf::Sprite{
                    .position    = r.position,
                    .scale       = {0.15f, 0.15f},
                    .origin      = txr->size / 2.f,
                    .rotation    = sf::radians(0.f),
                    .textureRect = *txr,
                    .color       = sf::Color::White,
                },
                sf::RenderStates{.texture = txAtlas});
        }
    }
};

} // namespace AOSImproved

////////////////////////////////////////////////////////////
namespace SOA
{
////////////////////////////////////////////////////////////
using ParticleSoA = SoAFor<sf::Vec2f, // position
                           sf::Vec2f, // velocity
                           sf::Vec2f, // acceleration

                           float, // lifetime
                           float, // lifetimeDecayRate

                           float, // scale
                           float, // scaleRate

                           float, // opacity
                           float, // opacityChange

                           float,  // rotation
                           float>; // angularVelocity

struct Field
{
    enum : sf::base::SizeT
    {
        Position,
        Velocity,
        Acceleration,

        Life,
        LifeDelta,

        Scale,
        ScaleDelta,

        Opacity,
        OpacityDelta,

        Rotation,
        RotationDelta
    };
};

////////////////////////////////////////////////////////////
struct Emitter
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float lifetime;
    float lifetimeDecayRate;

    float spawnTimer;
    float spawnRate;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::SizeT smokeEmitterIdx;
    sf::base::SizeT fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World
{
    sf::base::Vector<sf::base::Optional<Emitter>> smokeEmitters;
    sf::base::Vector<sf::base::Optional<Emitter>> fireEmitters;
    ParticleSoA                                   smokeParticles;
    ParticleSoA                                   fireParticles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT addSmokeEmitter(const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0; i < smokeEmitters.size(); ++i)
        {
            if (!smokeEmitters[i].hasValue())
            {
                smokeEmitters[i].emplace(emitter);
                return i;
            }
        }

        smokeEmitters.emplaceBack(emitter);
        return smokeEmitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT addFireEmitter(const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0; i < fireEmitters.size(); ++i)
        {
            if (!fireEmitters[i].hasValue())
            {
                fireEmitters[i].emplace(emitter);
                return i;
            }
        }

        fireEmitters.emplaceBack(emitter);
        return fireEmitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    void addSmokeParticle(const auto&... xs)
    {
        return smokeParticles.pushBack(xs...);
    }

    ////////////////////////////////////////////////////////////
    void addFireParticle(const auto&... xs)
    {
        return fireParticles.pushBack(xs...);
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        auto& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addSmokeEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 2.5f,
        });

        rocket.fireEmitterIdx = addFireEmitter({
            .lifetime          = 100.f,
            .lifetimeDecayRate = -0.01f,
            .spawnTimer        = 0.f,
            .spawnRate         = 1.25f,
        });

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        auto updateParticles = [&](auto& particles)
        {
            const auto nParticles = particles.getSize();

            auto&       positions     = particles.template get<Field::Position>();
            auto&       velocities    = particles.template get<Field::Velocity>();
            const auto& accelerations = particles.template get<Field::Acceleration>();

            auto&       lifes      = particles.template get<Field::Life>();
            const auto& lifeDeltas = particles.template get<Field::LifeDelta>();

            auto&       scales      = particles.template get<Field::Scale>();
            const auto& scaleDeltas = particles.template get<Field::ScaleDelta>();

            auto&       opacities     = particles.template get<Field::Opacity>();
            const auto& opacityDeltas = particles.template get<Field::OpacityDelta>();

            auto&       rotations      = particles.template get<Field::Rotation>();
            const auto& rotationDeltas = particles.template get<Field::RotationDelta>();

            for (sf::base::SizeT i = 0; i < nParticles; ++i)
            {
                velocities[i] += accelerations[i] * dt;
                positions[i] += velocities[i] * dt;

                lifes[i] += lifeDeltas[i] * dt;

                scales[i] += scaleDeltas[i] * dt;
                if (scales[i] <= 0.f)
                    scales[i] = 0.f;

                opacities[i] += opacityDeltas[i] * dt;
                if (opacities[i] <= 0.f)
                    opacities[i] = 0.f;

                rotations[i] += rotationDeltas[i] * dt;
            }
        };

        updateParticles(smokeParticles);
        updateParticles(fireParticles);

        for (auto& e : smokeEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->lifetime += e->lifetimeDecayRate * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                addSmokeParticle(
                    /* .position     */ e->position,
                    /* .velocity     */ rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                    /* .acceleration */ sf::Vec2f{0.f, -0.011f},

                    /* .lifetime          */ 1.f,
                    /* .lifetimeDecayRate */ -0.005f,

                    /* .scale     */ rng.getF(0.0025f, 0.0035f),
                    /* .scaleRate */ rng.getF(0.001f, 0.003f) * 2.75f,

                    /* .opacity       */ rng.getF(0.05f, 0.25f),
                    /* .opacityChange */ -rng.getF(0.001f, 0.002f) * 3.25f,

                    /* .rotation        */ rng.getF(0.f, 6.28f),
                    /* .angularVelocity */ rng.getF(-0.02f, 0.02f));
        }

        for (auto& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->lifetime += e->lifetimeDecayRate * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                addFireParticle(
                    /* .position     */ e->position,
                    /* .velocity     */ rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                    /* .acceleration */ sf::Vec2f{0.f, 0.07f},

                    /* .lifetime          */ 1.f,
                    /* .lifetimeDecayRate */ -0.005f,

                    /* .scale     */ rng.getF(0.5f, 0.7f) * 0.085f,
                    /* .scaleRate */ -rng.getF(0.001f, 0.003f) * 0.25f,

                    /* .opacity       */ rng.getF(0.2f, 0.4f) * 0.85f,
                    /* .opacityChange */ -0.001f,

                    /* .rotation        */ rng.getF(0.f, 6.28f),
                    /* .angularVelocity */ rng.getF(-0.002f, 0.002f));
        }

        for (auto& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (auto& se = smokeEmitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (auto& fe = fireEmitters[r.fireEmitterIdx])
                fe->position = r.position + sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        for (auto& e : smokeEmitters)
            if (e.hasValue() && e->lifetime <= 0.f)
                e.reset();

        for (auto& e : fireEmitters)
            if (e.hasValue() && e->lifetime <= 0.f)
                e.reset();

        smokeParticles.eraseIfBySwapping<Field::Life>([](const float lifetime) { return lifetime <= 0.f; });
        fireParticles.eraseIfBySwapping<Field::Life>([](const float lifetime) { return lifetime <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const auto& r)
        {
            const bool outOfBounds = r.position.x > 1680.f + 100.f;

            if (outOfBounds)
            {
                auto& se = smokeEmitters[r.smokeEmitterIdx];
                if (se.hasValue())
                    se.reset();

                auto& fe = fireEmitters[r.fireEmitterIdx];
                if (fe.hasValue())
                    fe.reset();
            }

            return outOfBounds;
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        smokeParticles.with<Field::Position, Field::Scale, Field::Opacity, Field::Rotation>(
            [&](const sf::Vec2f& position, const float& scale, const float& opacity, const float& rotation)
        { drawParticleImpl(rt, position, {scale, scale}, rotation, txrSmoke, opacity); });

        fireParticles.with<Field::Position, Field::Scale, Field::Opacity, Field::Rotation>(
            [&](const sf::Vec2f& position, const float& scale, const float& opacity, const float& rotation)
        { drawParticleImpl(rt, position, {scale, scale}, rotation, txrFire, opacity); });

        for (const auto& r : rockets)
        {
            auto* txr = &txrRocket;

            rt.draw(
                sf::Sprite{
                    .position    = r.position,
                    .scale       = {0.15f, 0.15f},
                    .origin      = txr->size / 2.f,
                    .rotation    = sf::radians(0.f),
                    .textureRect = *txr,
                    .color       = sf::Color::White,
                },
                sf::RenderStates{.texture = txAtlas});
        }
    }
};

} // namespace SOA

////////////////////////////////////////////////////////////
enum class Mode
{
    OOP,
    AOS,
    AOSImproved,
    SOA,
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    constexpr sf::Vec2f windowSize{1680.f, 1050.f};

    sf::RenderWindow window({
        .size            = windowSize.toVec2u(),
        .title           = "Rockets",
        .resizable       = false,
        .vsync           = false,
        .frametimeLimit  = 144u,
        .contextSettings = {.antiAliasingLevel = 8u},
    });

    // TODO P0: GPUStorage is still glitchy, some synchronization issue persists...
    window.setAutoBatchMode(sf::RenderTarget::AutoBatchMode::CPUStorage);

    //
    //
    // Set up imgui
    sf::ImGuiContext imGuiContext;

    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};
    txAtlas = &textureAtlas.getTexture();

    const auto addImgResourceToAtlas = [&](const sf::Path& path)
    { return textureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value(); };

    //
    //
    // Load fonts
    const auto fontTuffy        = sf::Font::openFromFile("resources/tuffy.ttf", &textureAtlas).value();
    const auto fontMouldyCheese = sf::Font::openFromFile("resources/mouldycheese.ttf", &textureAtlas).value();

    //
    //
    // Load images and add to texture atlas
    const sf::FloatRect spriteTextureRects[]{
        addImgResourceToAtlas("pCircle.png"),
        addImgResourceToAtlas("pExplosion.png"),
        addImgResourceToAtlas("pFire.png"),
        addImgResourceToAtlas("pSmoke.png"),
        addImgResourceToAtlas("pStar.png"),
        addImgResourceToAtlas("pTrail.png"),
        addImgResourceToAtlas("rocket.png"),
    };

    txrSmoke  = spriteTextureRects[3];
    txrFire   = spriteTextureRects[2];
    txrRocket = spriteTextureRects[6];

    //
    //
    // Options
    Mode  mode            = Mode::OOP;
    bool  drawStep        = true;
    float simulationSpeed = 0.1f;
    float rocketSpawnRate = 0.1f;
    float zoom            = 3.f;

    //
    //
    // Shared values
    float           rocketSpawnTimer = 0.f;
    sf::base::SizeT nTargetRockets   = 500;

    //
    //
    // OOP Particles
    OOP::World oopWorld;

    //
    //
    // AOS Particles
    AOS::World aosWorld;

    //
    //
    // AOSImproved Particles
    AOSImproved::World aosImprovedWorld;

    //
    //
    // SOA Particles
    SOA::World soaWorld;

    //
    //
    // Set up clock and time sampling
    sf::Clock clock;
    sf::Clock fpsClock;

    Sampler samplesUpdateMs(/* capacity */ 64u);
    Sampler samplesDrawMs(/* capacity */ 64u);
    Sampler samplesDisplayMs(/* capacity */ 64u);
    Sampler samplesFPS(/* capacity */ 64u);

    //
    //
    // Simulation loop
    while (true)
    {
        fpsClock.restart();

        ////////////////////////////////////////////////////////////
        // Event handling
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            while (sf::base::Optional event = window.pollEvent())
            {
                imGuiContext.processEvent(window, *event);

                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return 0;
            }
        }
        // ---

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            rocketSpawnTimer += rocketSpawnRate * simulationSpeed;

            if (mode == Mode::OOP)
            {
                sf::base::SizeT nRockets = 0;

                // TODO P0:
                /*
                for (const auto& entity : oopWorld.entities)
                {
                    if (dynamic_cast<OOP::Rocket*>(entity.get()))
                        ++nRockets;
                }
                */

                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    auto& rocket = oopWorld.addEntity<OOP::Rocket>();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.05f), 0.f};
                }

                oopWorld.update(simulationSpeed);
                oopWorld.cleanup();
            }
            else if (mode == Mode::AOS)
            {
                const sf::base::SizeT nRockets = aosWorld.rockets.size();

                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    aosWorld.addRocket({
                        .position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y}),
                        .velocity     = {},
                        .acceleration = {rng.getF(0.01f, 0.025f), 0.f},
                    });
                }

                aosWorld.update(simulationSpeed);
                aosWorld.cleanup();
            }
            else if (mode == Mode::AOSImproved)
            {
                const sf::base::SizeT nRockets = aosImprovedWorld.rockets.size();

                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    aosImprovedWorld.addRocket({
                        .position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y}),
                        .velocity     = {},
                        .acceleration = {rng.getF(0.01f, 0.025f), 0.f},
                    });
                }

                aosImprovedWorld.update(simulationSpeed);
                aosImprovedWorld.cleanup();
            }
            else if (mode == Mode::SOA)
            {
                const sf::base::SizeT nRockets = soaWorld.rockets.size();

                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    soaWorld.addRocket({
                        .position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y}),
                        .velocity     = {},
                        .acceleration = {rng.getF(0.01f, 0.025f), 0.f},
                    });
                }

                soaWorld.update(simulationSpeed);
                soaWorld.cleanup();
            }
        }
        samplesUpdateMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        ////////////////////////////////////////////////////////////
        // ImGui step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            imGuiContext.update(window, fpsClock.getElapsedTime());

            ImGui::Begin("TODO", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize({380.f, 510.f});

            const auto clearSamples = [&]
            {
                samplesUpdateMs.clear();
                samplesDrawMs.clear();
                samplesDisplayMs.clear();
                samplesFPS.clear();
            };

            const auto plotGraph = [&](const char* label, const char* unit, const Sampler& samples, float upperBound)
            {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "%.4f %s", samples.getAverage(), unit);
                ImGui::PlotLines(label, samples.data(), static_cast<int>(samples.size()), 0, buf, 0.f, upperBound, ImVec2{256.f, 32.f});
            };

            plotGraph("Update", " ms", samplesUpdateMs, 10.f);
            plotGraph("Draw", " ms", samplesDrawMs, 100.f);
            plotGraph("FPS", " FPS", samplesFPS, 300.f);
            plotGraph("Display", " ms", samplesDisplayMs, 300.f);

            if (mode == Mode::OOP)
            {
                ImGui::Text("Number of entities: %zu", oopWorld.entities.size());
            }
            else if (mode == Mode::AOS)
            {
                ImGui::Text("Number of entities: %zu",
                            aosWorld.rockets.size() + aosWorld.emitters.size() + aosWorld.particles.size());
            }
            else if (mode == Mode::AOSImproved)
            {
                ImGui::Text("Number of entities: %zu",
                            aosImprovedWorld.rockets.size() + aosImprovedWorld.smokeEmitters.size() +
                                aosImprovedWorld.smokeParticles.size() + aosImprovedWorld.fireEmitters.size() +
                                aosImprovedWorld.fireParticles.size());
            }
            else if (mode == Mode::SOA)
            {
                ImGui::Text("Number of entities: %zu",
                            soaWorld.rockets.size() + soaWorld.smokeEmitters.size() + soaWorld.smokeParticles.getSize() +
                                soaWorld.fireEmitters.size() + soaWorld.fireParticles.getSize());
            }

            ImGui::Combo("Mode", reinterpret_cast<int*>(&mode), "OOP\0AOS\0AOSImproved\0SOA\0");
            ImGui::Checkbox("Draw step", &drawStep);
            ImGui::SliderFloat("Simulation Speed", &simulationSpeed, 0.1f, 4.f);
            ImGui::SliderFloat("Rocket Spawn Rate", &rocketSpawnRate, 0.05f, 4.f);
            ImGui::SliderFloat("Zoom", &zoom, 0.1f, 4.f);

            ImGui::End();
        }
        // ---

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            window.clear();
            window.setView({.center = windowSize / 2.f, .size = windowSize / zoom});

            if (drawStep)
            {
                if (mode == Mode::OOP)
                    oopWorld.draw(window);
                else if (mode == Mode::AOS)
                    aosWorld.draw(window);
                else if (mode == Mode::AOSImproved)
                    aosImprovedWorld.draw(window);
                else if (mode == Mode::SOA)
                    soaWorld.draw(window);
            }
        }
        samplesDrawMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        // ---
        clock.restart();
        {
            imGuiContext.render(window);
            window.display();
        }
        samplesDisplayMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        samplesFPS.record(1.f / fpsClock.getElapsedTime().asSeconds());
    }
}
