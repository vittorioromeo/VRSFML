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

#include <string>


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
namespace OOP
{
////////////////////////////////////////////////////////////
struct World;

////////////////////////////////////////////////////////////
struct Entity
{
    World* world = nullptr;
    bool   alive = true;

    virtual ~Entity() = default;

    virtual void update(float)
    {
    }

    virtual void draw(sf::RenderTarget&)
    {
    }
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
        for (std::size_t i = 0; i < entities.size(); ++i)
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
struct PhysicalEntity : Entity
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    void update(float dt) override
    {
        position += velocity * dt;
        velocity += acceleration * dt;
    }
};

////////////////////////////////////////////////////////////
struct Emitter : PhysicalEntity
{
    float life;
    float lifeChange;

    void update(float dt) override
    {
        PhysicalEntity::update(dt);

        life += lifeChange * dt;

        if (life <= 0.f)
            alive = false;
    }

    void draw(sf::RenderTarget&) override
    {
    }
};

////////////////////////////////////////////////////////////
struct Particle : PhysicalEntity
{
    float life;
    float scale;
    float opacity;
    float rotation;

    float lifeChange;
    float scaleChange;
    float opacityChange;
    float rotationChange;

    void update(float dt) override
    {
        PhysicalEntity::update(dt);

        life += lifeChange * dt;
        scale += scaleChange * dt;
        opacity += opacityChange * dt;
        rotation += rotationChange * dt;

        if (life <= 0.f)
            alive = false;

        if (scale <= 0.f)
            scale = 0.f;

        if (opacity <= 0.f)
            opacity = 0.f;
    }

    void draw(sf::RenderTarget&) override
    {
    }
};

////////////////////////////////////////////////////////////
struct SmokeParticle : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        rt.draw(
            sf::Sprite{
                .position    = position,
                .scale       = {scale, scale},
                .origin      = txrSmoke.size / 2.f,
                .rotation    = sf::radians(rotation),
                .textureRect = txrSmoke,
                .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(opacity * 255.f)),
            },
            sf::RenderStates{.texture = txAtlas});
    }
};

////////////////////////////////////////////////////////////
struct FireParticle : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        rt.draw(
            sf::Sprite{
                .position    = position,
                .scale       = {scale, scale},
                .origin      = txrFire.size / 2.f,
                .rotation    = sf::radians(rotation),
                .textureRect = txrFire,
                .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(opacity * 255.f)),
            },
            sf::RenderStates{.texture = txAtlas});
    }
};

////////////////////////////////////////////////////////////
struct SmokeEmitter : Emitter
{
    void update(float dt) override
    {
        Emitter::update(dt);

        auto& p = world->addEntity<SmokeParticle>();

        p.position     = position;
        p.velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f;
        p.acceleration = {0.f, -0.002f};

        p.life     = 1.f;
        p.scale    = rng.getF(0.005f, 0.02f);
        p.opacity  = rng.getF(0.05f, 0.25f);
        p.rotation = rng.getF(0.f, 6.28f);

        p.lifeChange     = -0.001f;
        p.scaleChange    = rng.getF(0.001f, 0.003f) * 0.5f;
        p.opacityChange  = -rng.getF(0.001f, 0.002f) * 0.9f;
        p.rotationChange = rng.getF(-0.001f, 0.001f);
    }
};

////////////////////////////////////////////////////////////
struct FireEmitter : Emitter
{
    void update(float dt) override
    {
        Emitter::update(dt);

        auto& p = world->addEntity<FireParticle>();

        p.position     = position;
        p.velocity     = rng.getVec2f({-0.3f, -0.2f}, {0.3f, 0.f});
        p.acceleration = {0.f, 0.006f};

        p.life     = 1.f;
        p.scale    = rng.getF(0.01f, 0.03f) * 0.75f;
        p.opacity  = rng.getF(0.2f, 0.4f) * 0.85f;
        p.rotation = 0.f;

        p.lifeChange     = -0.003f;
        p.scaleChange    = rng.getF(0.001f, 0.003f) * 0.25f;
        p.opacityChange  = -0.0015f;
        p.rotationChange = rng.getF(-0.002f, 0.002f);
    }
};

////////////////////////////////////////////////////////////
struct Rocket : PhysicalEntity
{
    SmokeEmitter* smokeEmitter = nullptr;
    FireEmitter*  fireEmitter  = nullptr;

    bool init = false;

    void update(float dt) override
    {
        PhysicalEntity::update(dt);

        if (!init)
        {
            init = true;

            auto& se        = world->addEntity<SmokeEmitter>();
            se.position     = position;
            se.velocity     = {};
            se.acceleration = {};
            se.life         = 100.f;
            se.lifeChange   = -0.01f;
            smokeEmitter    = &se;

            auto& fe        = world->addEntity<FireEmitter>();
            fe.position     = position;
            fe.velocity     = {};
            fe.acceleration = {};
            fe.life         = 100.f;
            fe.lifeChange   = -0.01f;
            fireEmitter     = &fe;
        }

        velocity += acceleration * dt;
        position += velocity * dt;

        smokeEmitter->position = position - sf::Vec2f{12.f, 0.f};
        fireEmitter->position  = position - sf::Vec2f{12.f, 0.f};

        if (position.x > 1680.f + 100.f)
        {
            alive = false;

            smokeEmitter->alive = false;
            fireEmitter->alive  = false;
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

    float life;
    float lifeChange;

    ParticleType type;
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float life;
    float scale;
    float opacity;
    float rotation;

    float lifeChange;
    float scaleChange;
    float opacityChange;
    float rotationChange;

    ParticleType type;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    std::size_t smokeEmitterIdx;
    std::size_t fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World
{
    sf::base::Vector<sf::base::Optional<Emitter>> emitters;
    sf::base::Vector<Particle>                    particles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::size_t addEmitter(const Emitter& emitter)
    {
        for (std::size_t i = 0; i < emitters.size(); ++i)
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
    Rocket& addRocket()
    {
        auto& rocket = rockets.emplaceBack(Rocket{});

        rocket.smokeEmitterIdx = addEmitter(
            {.position = {}, .velocity = {}, .acceleration = {}, .life = 100.f, .lifeChange = -0.01f, .type = ParticleType::Smoke});

        rocket.fireEmitterIdx = addEmitter(
            {.position = {}, .velocity = {}, .acceleration = {}, .life = 100.f, .lifeChange = -0.01f, .type = ParticleType::Fire});

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        for (Particle& p : particles)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;

            p.life += p.lifeChange * dt;
            p.scale += p.scaleChange * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.rotationChange * dt;

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

            e->life += e->lifeChange * dt;

            if (e->type == ParticleType::Smoke)
            {
                addParticle(
                    {.position     = e->position,
                     .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                     .acceleration = {0.f, -0.002f},

                     .life     = 1.f,
                     .scale    = rng.getF(0.005f, 0.02f),
                     .opacity  = rng.getF(0.05f, 0.25f),
                     .rotation = rng.getF(0.f, 6.28f),

                     .lifeChange     = -0.001f,
                     .scaleChange    = rng.getF(0.001f, 0.003f) * 0.5f,
                     .opacityChange  = -rng.getF(0.001f, 0.002f) * 0.9f,
                     .rotationChange = rng.getF(-0.001f, 0.001f),

                     .type = ParticleType::Smoke});
            }
            else if (e->type == ParticleType::Fire)
            {
                addParticle({
                    .position     = e->position,
                    .velocity     = rng.getVec2f({-0.3f, -0.2f}, {0.3f, 0.f}),
                    .acceleration = {0.f, 0.006f},

                    .life     = 1.f,
                    .scale    = rng.getF(0.01f, 0.03f) * 0.75f,
                    .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                    .rotation = 0.f,

                    .lifeChange     = -0.003f,
                    .scaleChange    = rng.getF(0.001f, 0.003f) * 0.25f,
                    .opacityChange  = -0.0015f,
                    .rotationChange = rng.getF(-0.002f, 0.002f),

                    .type = ParticleType::Fire,
                });
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
            if (e.hasValue() && e->life <= 0.f)
                e.reset();

        sf::base::vectorSwapAndPopIf(particles, [](const auto& p) { return p.life <= 0.f; });

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

            rt.draw(
                sf::Sprite{
                    .position    = p.position,
                    .scale       = {p.scale, p.scale},
                    .origin      = txr->size / 2.f,
                    .rotation    = sf::radians(p.rotation),
                    .textureRect = *txr,
                    .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(p.opacity * 255.f)),
                },
                sf::RenderStates{.texture = txAtlas});
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

    float life;
    float lifeChange;
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float life;
    float scale;
    float opacity;
    float rotation;

    float lifeChange;
    float scaleChange;
    float opacityChange;
    float rotationChange;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    std::size_t smokeEmitterIdx;
    std::size_t fireEmitterIdx;
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
    [[nodiscard]] std::size_t addSmokeEmitter(const Emitter& emitter)
    {
        for (std::size_t i = 0; i < smokeEmitters.size(); ++i)
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
    [[nodiscard]] std::size_t addFireEmitter(const Emitter& emitter)
    {
        for (std::size_t i = 0; i < fireEmitters.size(); ++i)
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
        smokeParticles.emplaceBack(particle);
        return smokeParticles.back();
    }

    ////////////////////////////////////////////////////////////
    Particle& addFireParticle(const Particle& particle)
    {
        fireParticles.emplaceBack(particle);
        return fireParticles.back();
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket()
    {
        auto& rocket = rockets.emplaceBack(Rocket{});

        rocket.smokeEmitterIdx = addSmokeEmitter(
            {.position = {}, .velocity = {}, .acceleration = {}, .life = 100.f, .lifeChange = -0.01f});

        rocket.fireEmitterIdx = addFireEmitter(
            {.position = {}, .velocity = {}, .acceleration = {}, .life = 100.f, .lifeChange = -0.01f});

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        auto updateParticle = [&](Particle& p)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;

            p.life += p.lifeChange * dt;
            p.scale += p.scaleChange * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.rotationChange * dt;

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

            e->life += e->lifeChange * dt;

            addSmokeParticle({
                .position     = e->position,
                .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                .acceleration = {0.f, -0.002f},

                .life     = 1.f,
                .scale    = rng.getF(0.005f, 0.02f),
                .opacity  = rng.getF(0.05f, 0.25f),
                .rotation = rng.getF(0.f, 6.28f),

                .lifeChange     = -0.001f,
                .scaleChange    = rng.getF(0.001f, 0.003f) * 0.5f,
                .opacityChange  = -rng.getF(0.001f, 0.002f) * 0.9f,
                .rotationChange = rng.getF(-0.001f, 0.001f),
            });
        }

        for (auto& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->life += e->lifeChange * dt;

            addFireParticle({
                .position     = e->position,
                .velocity     = rng.getVec2f({-0.3f, -0.2f}, {0.3f, 0.f}),
                .acceleration = {0.f, 0.006f},

                .life     = 1.f,
                .scale    = rng.getF(0.01f, 0.03f) * 0.75f,
                .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                .rotation = 0.f,

                .lifeChange     = -0.003f,
                .scaleChange    = rng.getF(0.001f, 0.003f) * 0.25f,
                .opacityChange  = -0.0015f,
                .rotationChange = rng.getF(-0.002f, 0.002f),
            });
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
            if (e.hasValue() && e->life <= 0.f)
                e.reset();

        for (auto& e : fireEmitters)
            if (e.hasValue() && e->life <= 0.f)
                e.reset();

        sf::base::vectorSwapAndPopIf(smokeParticles, [](const auto& p) { return p.life <= 0.f; });
        sf::base::vectorSwapAndPopIf(fireParticles, [](const auto& p) { return p.life <= 0.f; });

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
        {
            rt.draw(
                sf::Sprite{
                    .position    = p.position,
                    .scale       = {p.scale, p.scale},
                    .origin      = txrSmoke.size / 2.f,
                    .rotation    = sf::radians(p.rotation),
                    .textureRect = txrSmoke,
                    .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(p.opacity * 255.f)),
                },
                sf::RenderStates{.texture = txAtlas});
        }

        for (const auto& p : fireParticles)
        {
            rt.draw(
                sf::Sprite{
                    .position    = p.position,
                    .scale       = {p.scale, p.scale},
                    .origin      = txrFire.size / 2.f,
                    .rotation    = sf::radians(p.rotation),
                    .textureRect = txrFire,
                    .color       = sf::Color::whiteMask(static_cast<sf::base::U8>(p.opacity * 255.f)),
                },
                sf::RenderStates{.texture = txAtlas});
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

} // namespace AOSImproved

////////////////////////////////////////////////////////////
enum class Mode
{
    OOP,
    AOS,
    AOSImproved,
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
        .size           = windowSize.toVec2u(),
        .title          = "Rockets",
        .resizable      = false,
        .vsync          = false,
        .frametimeLimit = 144u,
    });

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
    Mode mode     = Mode::OOP;
    bool drawStep = true;

    //
    //
    // Shared values
    std::size_t nTargetRockets = 500;

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
            if (mode == Mode::OOP)
            {
                std::size_t nRockets = 0;

                for (const auto& entity : oopWorld.entities)
                {
                    if (dynamic_cast<OOP::Rocket*>(entity.get()))
                        ++nRockets;
                }

                for (std::size_t i = 0; i < nTargetRockets - nRockets; ++i)
                {
                    auto& rocket = oopWorld.addEntity<OOP::Rocket>();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.05f), 0.f};
                }

                oopWorld.update(1.f);
                oopWorld.cleanup();
            }
            else if (mode == Mode::AOS)
            {
                const std::size_t nRockets = aosWorld.rockets.size();

                for (std::size_t i = 0; i < nTargetRockets - nRockets; ++i)
                {
                    auto& rocket = aosWorld.addRocket();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.05f), 0.f};
                }

                aosWorld.update(1.f);
                aosWorld.cleanup();
            }
            else if (mode == Mode::AOSImproved)
            {
                const std::size_t nRockets = aosImprovedWorld.rockets.size();

                for (std::size_t i = 0; i < nTargetRockets - nRockets; ++i)
                {
                    auto& rocket = aosImprovedWorld.addRocket();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.05f), 0.f};
                }

                aosImprovedWorld.update(1.f);
                aosImprovedWorld.cleanup();
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

            ImGui::Begin("Vittorio's SFML fork: particles example", nullptr, ImGuiWindowFlags_NoResize);
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
                ImGui::PlotLines(label,
                                 samples.data(),
                                 static_cast<int>(samples.size()),
                                 0,
                                 (std::to_string(samples.getAverage()) + unit).c_str(),
                                 0.f,
                                 upperBound,
                                 ImVec2{256.f, 32.f});
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

            ImGui::Combo("Mode", reinterpret_cast<int*>(&mode), "OOP\0AOS\0AOSImproved\0");
            ImGui::Checkbox("Draw step", &drawStep);

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

            if (drawStep)
            {
                if (mode == Mode::OOP)
                    oopWorld.draw(window);
                else if (mode == Mode::AOS)
                    aosWorld.draw(window);
                else if (mode == Mode::AOSImproved)
                    aosImprovedWorld.draw(window);
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
