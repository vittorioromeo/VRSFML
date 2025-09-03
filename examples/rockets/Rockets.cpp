#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/SoA.hpp"     // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DefaultShader.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glsl.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Shader.hpp"
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
#include "SFML/Base/Builtins/OffsetOf.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vertex instancedQuadVertices[4] = {
    {{-0.5, -0.5}, sf::Color::White, {0.f, 0.f}}, // Top-left
    {{0.5, -0.5}, sf::Color::White, {1.f, 0.f}},  // Top-right
    {{0.5, 0.5}, sf::Color::White, {1.f, 1.f}},   // Bottom-right
    {{-0.5, 0.5}, sf::Color::White, {0.f, 1.f}}   // Bottom-left
};


////////////////////////////////////////////////////////////
constexpr unsigned int instancedQuadIndices[6] = {
    0,
    1,
    2,
    2,
    3,
    0,
};


////////////////////////////////////////////////////////////
struct ParticleInstanceData
{
    sf::Vec2f position;
    float     scale;
    float     rotation;
    float     opacity;
};


////////////////////////////////////////////////////////////
constexpr const char* instancedVertexShader = R"glsl(

layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform sampler2D sf_u_texture;
layout(location = 2) uniform vec4 u_texRect;
layout(location = 3) uniform vec2 u_invTexSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color; // Unused but part of Vertex struct
layout(location = 2) in vec2 sf_a_texCoord;

// Per-instance attributes (unique for each sprite)
layout(location = 3) in vec2 instance_position;
layout(location = 4) in float instance_scale;
layout(location = 5) in float instance_rotation;
layout(location = 6) in float instance_opacity;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    // scale in local (pixel) space
    vec2 local = sf_a_position * u_texRect.zw;

    // inline rotate + scale
    float c = cos(instance_rotation);
    float s = sin(instance_rotation);
    float x = local.x * c - local.y * s;
    float y = local.x * s + local.y * c;
    vec2 worldPos = instance_position + instance_scale * vec2(x, y);

    gl_Position = sf_u_mvpMatrix * vec4(worldPos, 0.0, 1.0);

    sf_v_color = vec4(1.0, 1.0, 1.0, instance_opacity);

    vec2 final_texCoord = u_texRect.xy + (sf_a_texCoord * u_texRect.zw);
    sf_v_texCoord = final_texCoord * u_invTexSize;
}

)glsl";


////////////////////////////////////////////////////////////
sf::Shader*                            instanceRenderingShader         = nullptr;
const sf::Shader::UniformLocation*     instanceRenderingULTextureRect  = nullptr;
const sf::Shader::UniformLocation*     instanceRenderingInvTextureSize = nullptr;
sf::RenderTarget::VAOHandle*           instanceRenderingVAOGroup       = nullptr;
sf::RenderTarget::VBOHandle*           instanceRenderingVBOs[8]        = {};
sf::base::Vector<ParticleInstanceData> instanceRenderingDataBuffer;


////////////////////////////////////////////////////////////
RNGFast rng;


////////////////////////////////////////////////////////////
sf::Texture*  txAtlas = nullptr;
sf::FloatRect txrSmoke;
sf::FloatRect txrFire;
sf::FloatRect txrRocket;


////////////////////////////////////////////////////////////
[[gnu::always_inline]] void drawParticleImpl(
    sf::RenderTarget&   rt,
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
[[gnu::always_inline]] void drawRocketImpl(sf::RenderTarget& rt, const sf::Vec2f position)
{
    rt.draw(
        sf::Sprite{
            .position    = position,
            .scale       = {0.15f, 0.15f},
            .origin      = txrRocket.size / 2.f,
            .rotation    = sf::radians(0.f),
            .textureRect = txrRocket,
            .color       = sf::Color::White,
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
        // Cannot use range-based `for` as new elements are created while updating
        for (sf::base::SizeT i = 0u; i < entities.size(); ++i) // NOLINT(modernize-loop-convert)
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
    float spawnTimer;
    float spawnRate;

    virtual void spawnParticle() = 0;

    void update(float dt) override
    {
        Entity::update(dt);

        spawnTimer += spawnRate * dt;

        for (; spawnTimer >= 1.f; spawnTimer -= 1.f)
            spawnParticle();
    }
};

////////////////////////////////////////////////////////////
struct Particle : Entity
{
    float scale;
    float opacity;
    float rotation;

    float scaleRate;
    float opacityChange;
    float angularVelocity;

    void update(float dt) override
    {
        Entity::update(dt);

        scale += scaleRate * dt;
        opacity += opacityChange * dt;
        rotation += angularVelocity * dt;

        alive = opacity > 0.f;
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

        p.scale    = rng.getF(0.0025f, 0.0035f);
        p.opacity  = rng.getF(0.05f, 0.25f);
        p.rotation = rng.getF(0.f, 6.28f);

        p.scaleRate       = rng.getF(0.001f, 0.003f) * 2.75f;
        p.opacityChange   = -rng.getF(0.001f, 0.002f) * 3.25f;
        p.angularVelocity = rng.getF(-0.02f, 0.02f);
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

        p.scale    = rng.getF(0.5f, 0.7f) * 0.085f;
        p.opacity  = rng.getF(0.2f, 0.4f) * 0.85f;
        p.rotation = rng.getF(0.f, 6.28f);

        p.scaleRate       = -rng.getF(0.001f, 0.003f) * 0.25f;
        p.opacityChange   = -0.001f;
        p.angularVelocity = rng.getF(-0.002f, 0.002f);
    }
};

////////////////////////////////////////////////////////////
struct Rocket : Entity
{
    SmokeEmitter* smokeEmitter = nullptr;
    FireEmitter*  fireEmitter  = nullptr;

    void init()
    {
        smokeEmitter               = &world->addEntity<SmokeEmitter>();
        smokeEmitter->position     = position;
        smokeEmitter->velocity     = {};
        smokeEmitter->acceleration = {};
        smokeEmitter->spawnTimer   = 0.f;
        smokeEmitter->spawnRate    = 2.5f;

        fireEmitter               = &world->addEntity<FireEmitter>();
        fireEmitter->position     = position;
        fireEmitter->velocity     = {};
        fireEmitter->acceleration = {};
        fireEmitter->spawnTimer   = 0.f;
        fireEmitter->spawnRate    = 1.25f;
    }

    void update(float dt) override
    {
        Entity::update(dt);

        smokeEmitter->position = position - sf::Vec2f{12.f, 0.f};
        fireEmitter->position  = position - sf::Vec2f{12.f, 0.f};

        if (position.x > 1680.f + 64.f)
        {
            alive = false;

            smokeEmitter->alive = false;
            fireEmitter->alive  = false;
        }
    }

    void draw(sf::RenderTarget& rt) override
    {
        drawRocketImpl(rt, position);
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

    float scale;
    float opacity;
    float rotation;

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
        for (sf::base::SizeT i = 0u; i < emitters.size(); ++i)
            if (!emitters[i].hasValue())
            {
                emitters[i].emplace(emitter);
                return i;
            }

        emitters.emplaceBack(emitter);
        return emitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        Rocket& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addEmitter({
            .spawnTimer = 0.f,
            .spawnRate  = 2.5f,
            .type       = ParticleType::Smoke,
        });

        rocket.fireEmitterIdx = addEmitter({
            .spawnTimer = 0.f,
            .spawnRate  = 1.25f,
            .type       = ParticleType::Fire,
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

            p.scale += p.scaleRate * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.angularVelocity * dt;
        }

        for (sf::base::Optional<Emitter>& e : emitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
            {
                if (e->type == ParticleType::Smoke)
                {
                    particles.pushBack(
                        {.position     = e->position,
                         .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                         .acceleration = {0.f, -0.011f},

                         .scale    = rng.getF(0.0025f, 0.0035f),
                         .opacity  = rng.getF(0.05f, 0.25f),
                         .rotation = rng.getF(0.f, 6.28f),

                         .scaleRate       = rng.getF(0.001f, 0.003f) * 2.75f,
                         .opacityChange   = -rng.getF(0.001f, 0.002f) * 3.25f,
                         .angularVelocity = rng.getF(-0.02f, 0.02f),

                         .type = ParticleType::Smoke});
                }
                else if (e->type == ParticleType::Fire)
                {
                    particles.pushBack({
                        .position     = e->position,
                        .velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                        .acceleration = {0.f, 0.07f},

                        .scale    = rng.getF(0.5f, 0.7f) * 0.085f,
                        .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                        .rotation = rng.getF(0.f, 6.28f),

                        .scaleRate       = -rng.getF(0.001f, 0.003f) * 0.25f,
                        .opacityChange   = -0.001f,
                        .angularVelocity = rng.getF(-0.002f, 0.002f),

                        .type = ParticleType::Fire,
                    });
                }
            }
        }

        for (Rocket& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (sf::base::Optional<Emitter>& se = emitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (sf::base::Optional<Emitter>& fe = emitters[r.fireEmitterIdx])
                fe->position = r.position - sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        sf::base::vectorSwapAndPopIf(particles, [](const Particle& p) { return p.opacity <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const Rocket& r)
        {
            if (r.position.x <= 1680.f + 64.f)
                return false;

            emitters[r.smokeEmitterIdx].reset();
            emitters[r.fireEmitterIdx].reset();

            return true; // Out of bounds
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        for (const Particle& p : particles)
        {
            auto* txr = (p.type == ParticleType::Smoke) ? &txrSmoke : &txrFire;
            drawParticleImpl(rt, p.position, {p.scale, p.scale}, p.rotation, *txr, p.opacity);
        }

        for (const Rocket& r : rockets)
            drawRocketImpl(rt, r.position);
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

    float spawnTimer;
    float spawnRate;
};

////////////////////////////////////////////////////////////
struct Particle
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float scale;
    float opacity;
    float rotation;

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

    sf::base::U16 smokeEmitterIdx;
    sf::base::U16 fireEmitterIdx;
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
    [[nodiscard]] sf::base::SizeT addEmitter(sf::base::Vector<sf::base::Optional<Emitter>>& emitters, const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0u; i < emitters.size(); ++i)
            if (!emitters[i].hasValue())
            {
                emitters[i].emplace(emitter);
                return i;
            }

        emitters.emplaceBack(emitter);
        return emitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        Rocket& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addEmitter(smokeEmitters, {.spawnTimer = 0.f, .spawnRate = 2.5f});
        rocket.fireEmitterIdx  = addEmitter(fireEmitters, {.spawnTimer = 0.f, .spawnRate = 1.25f});

        return rocket;
    }

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        auto updateParticle = [&](Particle& p)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;

            p.scale += p.scaleRate * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.angularVelocity * dt;
        };

        for (Particle& p : smokeParticles)
            updateParticle(p);

        for (Particle& p : fireParticles)
            updateParticle(p);

        for (sf::base::Optional<Emitter>& e : smokeEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                smokeParticles.pushBack({
                    .position     = e->position,
                    .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                    .acceleration = {0.f, -0.011f},

                    .scale    = rng.getF(0.0025f, 0.0035f),
                    .opacity  = rng.getF(0.05f, 0.25f),
                    .rotation = rng.getF(0.f, 6.28f),

                    .scaleRate       = rng.getF(0.001f, 0.003f) * 2.75f,
                    .opacityChange   = -rng.getF(0.001f, 0.002f) * 3.25f,
                    .angularVelocity = rng.getF(-0.02f, 0.02f),
                });
        }

        for (sf::base::Optional<Emitter>& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                fireParticles.pushBack({
                    .position     = e->position,
                    .velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                    .acceleration = {0.f, 0.07f},

                    .scale    = rng.getF(0.5f, 0.7f) * 0.085f,
                    .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
                    .rotation = rng.getF(0.f, 6.28f),

                    .scaleRate       = -rng.getF(0.001f, 0.003f) * 0.25f,
                    .opacityChange   = -0.001f,
                    .angularVelocity = rng.getF(-0.002f, 0.002f),
                });
        }

        for (Rocket& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (sf::base::Optional<Emitter>& se = smokeEmitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (sf::base::Optional<Emitter>& fe = fireEmitters[r.fireEmitterIdx])
                fe->position = r.position - sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        sf::base::vectorSwapAndPopIf(smokeParticles, [](const Particle& p) { return p.opacity <= 0.f; });
        sf::base::vectorSwapAndPopIf(fireParticles, [](const Particle& p) { return p.opacity <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const Rocket& r)
        {
            if (r.position.x <= 1680.f + 64.f)
                return false;

            smokeEmitters[r.smokeEmitterIdx].reset();
            fireEmitters[r.fireEmitterIdx].reset();

            return true; // Out of bounds
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        const auto drawParticlesInstanced =
            [&](const sf::base::SizeT vboIndexOffset, const sf::FloatRect& txr, const auto& particles)
        {
            const auto nParticles = particles.size();

            instanceRenderingDataBuffer.clear();
            instanceRenderingDataBuffer.reserve(nParticles);

            for (sf::base::SizeT i = 0u; i < nParticles; ++i)
                instanceRenderingDataBuffer
                    .emplaceBack(particles[i].position, particles[i].scale, particles[i].rotation, particles[i].opacity);

            auto setupSpriteInstanceAttribs = [&](sf::RenderTarget::InstanceAttributeBinder& binder)
            {
                using IAB = sf::RenderTarget::InstanceAttributeBinder;

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset]);
                binder.uploadContiguousData(nParticles, instanceRenderingDataBuffer.data());

                constexpr auto stride = sizeof(ParticleInstanceData);

                binder.setup(3, 2, IAB::Type::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, position));
                binder.setup(4, 1, IAB::Type::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, scale));
                binder.setup(5, 1, IAB::Type::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, rotation));
                binder.setup(6, 1, IAB::Type::Float, true, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, opacity));
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.immediateDrawInstancedIndexedVertices(*instanceRenderingVAOGroup,
                                                     instancedQuadVertices,
                                                     4,
                                                     instancedQuadIndices,
                                                     6,
                                                     nParticles,
                                                     sf::PrimitiveType::Triangles,
                                                     {.texture = txAtlas, .shader = instanceRenderingShader},
                                                     setupSpriteInstanceAttribs);
        };

        drawParticlesInstanced(0, txrSmoke, smokeParticles);
        drawParticlesInstanced(1, txrFire, fireParticles);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
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

    float spawnTimer;
    float spawnRate;
};

////////////////////////////////////////////////////////////
struct Rocket
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::U16 smokeEmitterIdx;
    sf::base::U16 fireEmitterIdx;
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
    [[nodiscard]] sf::base::SizeT addEmitter(sf::base::Vector<sf::base::Optional<Emitter>>& emitters, const Emitter& emitter)
    {
        for (sf::base::SizeT i = 0u; i < emitters.size(); ++i)
            if (!emitters[i].hasValue())
            {
                emitters[i].emplace(emitter);
                return i;
            }

        emitters.emplaceBack(emitter);
        return emitters.size() - 1;
    }

    ////////////////////////////////////////////////////////////
    Rocket& addRocket(const Rocket& r)
    {
        Rocket& rocket = rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = addEmitter(smokeEmitters, {.spawnTimer = 0.f, .spawnRate = 2.5f});
        rocket.fireEmitterIdx  = addEmitter(fireEmitters, {.spawnTimer = 0.f, .spawnRate = 1.25f});

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

            auto&       scales      = particles.template get<Field::Scale>();
            const auto& scaleDeltas = particles.template get<Field::ScaleDelta>();

            auto&       opacities     = particles.template get<Field::Opacity>();
            const auto& opacityDeltas = particles.template get<Field::OpacityDelta>();

            auto&       rotations      = particles.template get<Field::Rotation>();
            const auto& rotationDeltas = particles.template get<Field::RotationDelta>();

            for (sf::base::SizeT i = 0u; i < nParticles; ++i)
            {
                velocities[i] += accelerations[i] * dt;
                positions[i] += velocities[i] * dt;
                scales[i] += scaleDeltas[i] * dt;
                opacities[i] += opacityDeltas[i] * dt;
                rotations[i] += rotationDeltas[i] * dt;
            }
        };

        updateParticles(smokeParticles);
        updateParticles(fireParticles);

        for (sf::base::Optional<Emitter>& e : smokeEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                smokeParticles.pushBack(
                    /* .position     */ e->position,
                    /* .velocity     */ rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
                    /* .acceleration */ sf::Vec2f{0.f, -0.011f},

                    /* .scale     */ rng.getF(0.0025f, 0.0035f),
                    /* .scaleRate */ rng.getF(0.001f, 0.003f) * 2.75f,

                    /* .opacity       */ rng.getF(0.05f, 0.25f),
                    /* .opacityChange */ -rng.getF(0.001f, 0.002f) * 3.25f,

                    /* .rotation        */ rng.getF(0.f, 6.28f),
                    /* .angularVelocity */ rng.getF(-0.02f, 0.02f));
        }

        for (sf::base::Optional<Emitter>& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;

            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                fireParticles.pushBack(
                    /* .position     */ e->position,
                    /* .velocity     */ rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
                    /* .acceleration */ sf::Vec2f{0.f, 0.07f},

                    /* .scale     */ rng.getF(0.5f, 0.7f) * 0.085f,
                    /* .scaleRate */ -rng.getF(0.001f, 0.003f) * 0.25f,

                    /* .opacity       */ rng.getF(0.2f, 0.4f) * 0.85f,
                    /* .opacityChange */ -0.001f,

                    /* .rotation        */ rng.getF(0.f, 6.28f),
                    /* .angularVelocity */ rng.getF(-0.002f, 0.002f));
        }

        for (Rocket& r : rockets)
        {
            r.position += r.velocity * dt;
            r.velocity += r.acceleration * dt;

            if (sf::base::Optional<Emitter>& se = smokeEmitters[r.smokeEmitterIdx])
                se->position = r.position - sf::Vec2f{12.f, 0.f};

            if (sf::base::Optional<Emitter>& fe = fireEmitters[r.fireEmitterIdx])
                fe->position = r.position - sf::Vec2f{12.f, 0.f};
        }
    }

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        smokeParticles.eraseIfBySwapping<Field::Opacity>([](const float opacity) { return opacity <= 0.f; });
        fireParticles.eraseIfBySwapping<Field::Opacity>([](const float opacity) { return opacity <= 0.f; });

        sf::base::vectorSwapAndPopIf(rockets,
                                     [&](const Rocket& r)
        {
            if (r.position.x <= 1680.f + 64.f)
                return false;

            smokeEmitters[r.smokeEmitterIdx].reset();
            fireEmitters[r.fireEmitterIdx].reset();

            return true; // Out of bounds
        });
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& rt)
    {
        const auto drawParticlesInstanced =
            [&](const sf::base::SizeT vboIndexOffset, const sf::FloatRect& txr, const auto& particles)
        {
            const auto nParticles = particles.getSize();

            auto setupSpriteInstanceAttribs = [&](sf::RenderTarget::InstanceAttributeBinder& binder)
            {
                using IAB = sf::RenderTarget::InstanceAttributeBinder;

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 0]);
                binder.uploadContiguousData(nParticles, particles.template get<Field::Position>().data());
                binder.setup(3, 2, IAB::Type::Float, false, sizeof(sf::Vec2f), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 1]);
                binder.uploadContiguousData(nParticles, particles.template get<Field::Scale>().data());
                binder.setup(4, 1, IAB::Type::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 2]);
                binder.uploadContiguousData(nParticles, particles.template get<Field::Rotation>().data());
                binder.setup(5, 1, IAB::Type::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 3]);
                binder.uploadContiguousData(nParticles, particles.template get<Field::Opacity>().data());
                binder.setup(6, 1, IAB::Type::Float, true, sizeof(float), 0u);
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.immediateDrawInstancedIndexedVertices(*instanceRenderingVAOGroup,
                                                     instancedQuadVertices,
                                                     4,
                                                     instancedQuadIndices,
                                                     6,
                                                     nParticles,
                                                     sf::PrimitiveType::Triangles,
                                                     {.texture = txAtlas, .shader = instanceRenderingShader},
                                                     setupSpriteInstanceAttribs);
        };

        drawParticlesInstanced(0, txrSmoke, smokeParticles);
        drawParticlesInstanced(4, txrFire, fireParticles);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
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
        .contextSettings = {.antiAliasingLevel = 16u},
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
    sf::TextureAtlas textureAtlas{sf::Texture::create({512u, 256u}, {.smooth = true}).value()};
    txAtlas = &textureAtlas.getTexture();

    const auto addImgResourceToAtlas = [&](const sf::Path& path)
    { return textureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value(); };

    //
    //
    // Load fonts
    ImFont* const fontImGuiGeistMono{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/geistmono.ttf", 32.f)};

    //
    //
    // Load images and add to texture atlas
    txrSmoke  = addImgResourceToAtlas("pSmoke.png");
    txrFire   = addImgResourceToAtlas("pFire.png");
    txrRocket = addImgResourceToAtlas("rocket.png");

    //
    //
    // Instanced rendering setup
    // TODO P0: cleanup
    auto instancedRenderingShaderImpl = sf::Shader::loadFromMemory({.vertexCode   = instancedVertexShader,
                                                                    .fragmentCode = sf::DefaultShader::srcFragment})
                                            .value();
    instanceRenderingShader = &instancedRenderingShaderImpl;

    auto instancedRenderingVAOGroupImpl = sf::RenderTarget::VAOHandle{};
    instanceRenderingVAOGroup           = &instancedRenderingVAOGroupImpl;

    sf::RenderTarget::VBOHandle instancedRenderingVBOsImpl[8];
    for (sf::base::SizeT i = 0u; i < 8u; ++i)
        instanceRenderingVBOs[i] = &instancedRenderingVBOsImpl[i];

    const auto instanceRenderingULTextureRectImpl = instancedRenderingShaderImpl.getUniformLocation("u_texRect").value();
    instanceRenderingULTextureRect = &instanceRenderingULTextureRectImpl;

    const auto instanceRenderingInvTextureSizeImpl = instancedRenderingShaderImpl.getUniformLocation("u_invTexSize").value();
    instanceRenderingInvTextureSize = &instanceRenderingInvTextureSizeImpl;

    instanceRenderingShader->setUniform(*instanceRenderingInvTextureSize, 1.f / txAtlas->getSize().toVec2f());

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
    // Simulation state
    float              rocketSpawnTimer = 0.f;
    OOP::World         oopWorld;
    AOS::World         aosWorld;
    AOSImproved::World aosImprovedWorld;
    SOA::World         soaWorld;

    oopWorld.entities.reserve(1'250'000);

    aosWorld.rockets.reserve(2500);
    aosWorld.emitters.reserve(5000);
    aosWorld.particles.reserve(1'000'000);

    aosImprovedWorld.rockets.reserve(2500);
    aosImprovedWorld.smokeEmitters.reserve(2500);
    aosImprovedWorld.fireEmitters.reserve(2500);
    aosImprovedWorld.smokeParticles.reserve(750'000);
    aosImprovedWorld.fireParticles.reserve(750'000);

    soaWorld.rockets.reserve(2500);
    soaWorld.smokeEmitters.reserve(2500);
    soaWorld.fireEmitters.reserve(2500);
    soaWorld.smokeParticles.reserve(750'000);
    soaWorld.fireParticles.reserve(750'000);

    //
    //
    // Set up clock and time sampling
    sf::Clock clock;
    sf::Clock fpsClock;

    Sampler samplesUpdateMs(/* capacity */ 128u);
    Sampler samplesDrawMs(/* capacity */ 128u);
    Sampler samplesDisplayMs(/* capacity */ 128u);
    Sampler samplesFPS(/* capacity */ 128u);

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

                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (eKeyPressed->code == sf::Keyboard::Key::Num1)
                    {
                        mode            = Mode::OOP;
                        drawStep        = true;
                        simulationSpeed = 0.1f;
                        rocketSpawnRate = 0.f;
                        zoom            = 3.f;

                        oopWorld = {};

                        for (int k = 0; k < 6; ++k)
                        {
                            {
                                auto& r        = oopWorld.addEntity<OOP::Rocket>();
                                r.position     = {-256.f * static_cast<float>(k) + -16.f, windowSize.y / 2.f};
                                r.velocity     = {1.0f, 0.f};
                                r.acceleration = {0.03f, 0.f};
                            }

                            for (int i = 1; i <= 5; ++i)
                            {
                                auto& r0    = oopWorld.addEntity<OOP::Rocket>();
                                r0.position = {-256.f * static_cast<float>(k) - 16.f + -48.f * static_cast<float>(i),
                                               windowSize.y / 2.f - 32.f * static_cast<float>(i)};
                                r0.velocity = {1.0f, 0.f};
                                r0.acceleration = {0.03f, 0.f};

                                auto& r1    = oopWorld.addEntity<OOP::Rocket>();
                                r1.position = {-256.f * static_cast<float>(k) - 16.f + -48.f * static_cast<float>(i),
                                               windowSize.y / 2.f + 32.f * static_cast<float>(i)};
                                r1.velocity = {1.0f, 0.f};
                                r1.acceleration = {0.03f, 0.f};
                            }
                        }
                    }
                }
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
                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    auto& rocket = oopWorld.addEntity<OOP::Rocket>();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, windowSize.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.025f), 0.f};

                    rocket.init();
                }

                oopWorld.update(simulationSpeed);
                oopWorld.cleanup();
            }
            else if (mode == Mode::AOS)
            {
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


            const auto plotGraphNoOverlay = [&](const char* label, const Sampler& samples, float upperBound)
            {
                ImGui::PlotLines(label,
                                 samples.data(),
                                 static_cast<int>(samples.size()),
                                 0,
                                 nullptr,
                                 0.f,
                                 upperBound,
                                 ImVec2{128.f, 32.f});
            };

            ImGui::SetNextWindowSize(ImVec2{440.f, 470.f});
            ImGui::SetNextWindowPos({windowSize.x - 440.f - 24.f, 24.f});

            ImGui::PushFont(fontImGuiGeistMono);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f); // Set corner radius

            ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowFontScale(1.25f);

            plotGraphNoOverlay("##infofps", samplesFPS, 144.f);
            ImGui::SameLine();
            ImGui::Text("    FPS: %.1f", samplesFPS.getAverage());

            plotGraphNoOverlay("##infoupdate", samplesUpdateMs, 30.f);
            ImGui::SameLine();
            ImGui::Text(" Update: %.1f ms", samplesUpdateMs.getAverage());

            plotGraphNoOverlay("##infodraw", samplesDrawMs, 30.f);
            ImGui::SameLine();
            ImGui::Text("   Draw: %.1f ms", samplesDrawMs.getAverage());

            ImGui::Separator();
            ImGui::SetWindowFontScale(1.f);

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

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.75f);
            ImGui::Checkbox("Enable rendering", &drawStep);

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.75f);
            ImGui::Combo("##infoMode",
                         reinterpret_cast<int*>(&mode),
                         "OOP (Heap-allocated entities)\0AoS (Very branchy)\0AoS (Improved)\0SoA\0");

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.75f);

            ImGui::Text("Rocket spawn rate: %.1fx", static_cast<double>(rocketSpawnRate));

            if (ImGui::Button("x1.0##r1"))
                rocketSpawnRate = 1.f;
            else if (ImGui::SameLine(), ImGui::Button("x2.0##r2"))
                rocketSpawnRate = 2.f;
            else if (ImGui::SameLine(), ImGui::Button("x3.0##r3"))
                rocketSpawnRate = 3.f;
            else if (ImGui::SameLine(), ImGui::Button("x4.0##r4"))
                rocketSpawnRate = 4.f;
            else if (ImGui::SameLine(), ImGui::Button("x5.0##r5"))
                rocketSpawnRate = 5.f;

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.75f);

            ImGui::Text("Simulation speed: %.1fx", static_cast<double>(simulationSpeed));

            if (ImGui::Button("x0.1##s1"))
                simulationSpeed = 0.1f;
            else if (ImGui::SameLine(), ImGui::Button("x0.5##s2"))
                simulationSpeed = 0.5f;
            else if (ImGui::SameLine(), ImGui::Button("x1.0##s3"))
                simulationSpeed = 1.f;
            else if (ImGui::SameLine(), ImGui::Button("x2.0##s4"))
                simulationSpeed = 2.f;
            else if (ImGui::SameLine(), ImGui::Button("x3.0##s5"))
                simulationSpeed = 3.f;
            else if (ImGui::SameLine(), ImGui::Button("x4.0##s6"))
                simulationSpeed = 4.f;
            else if (ImGui::SameLine(), ImGui::Button("x5.0##s7"))
                simulationSpeed = 5.f;

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.75f);

            ImGui::Text("Zoom level: %.1fx", static_cast<double>(zoom));
            ImGui::SliderFloat("##Zoom", &zoom, 1.f, 3.f);

            ImGui::End();

            ImGui::PopStyleVar();
            ImGui::PopFont();
        }
        // ---

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            window.clear();
            window.setView({.center = {windowSize.x / (2.f * zoom), windowSize.y / 2.f}, .size = windowSize / zoom});

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
