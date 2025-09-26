#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "SoAPFR.hpp"                 // TODO P1: avoid the relative path...?

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
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

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

#include "ExampleUtils.hpp"

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
constexpr unsigned int instancedQuadIndices[6] = {0u, 1u, 2u, 2u, 3u, 0u};


////////////////////////////////////////////////////////////
struct ParticleInstanceData // NOLINT(cppcoreguidelines-pro-type-member-init)
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
layout(location = 1) in vec4 sf_a_color; // Unused but part of `sf::Vertex` struct
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
sf::base::Vector<ParticleInstanceData> instanceRenderingDataBuffer[2];


////////////////////////////////////////////////////////////
RNGFast rng;


////////////////////////////////////////////////////////////
sf::Texture*  txAtlas = nullptr;
sf::FloatRect txrSmoke;
sf::FloatRect txrFire;
sf::FloatRect txrRocket;


////////////////////////////////////////////////////////////
[[nodiscard]] sf::RenderTarget::DrawInstancedIndexedVerticesSettings makeInstancedDrawSettings(const sf::base::SizeT nInstances)
{
    return {.vaoHandle     = *instanceRenderingVAOGroup,
            .vertexData    = instancedQuadVertices,
            .vertexCount   = 4u,
            .indexData     = instancedQuadIndices,
            .indexCount    = 6u,
            .instanceCount = nInstances,
            .primitiveType = sf::PrimitiveType::Triangles,
            .renderStates  = {.texture = txAtlas, .shader = instanceRenderingShader}};
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline void drawParticleImpl(
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
[[gnu::always_inline]] inline void drawRocketImpl(sf::RenderTarget& rt, const sf::Vec2f position)
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
struct Emitter : Entity // NOLINT(cppcoreguidelines-pro-type-member-init)
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
struct Particle : Entity // NOLINT(cppcoreguidelines-pro-type-member-init)
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
struct SmokeParticle final : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        drawParticleImpl(rt, position, {scale, scale}, rotation, txrSmoke, opacity);
    }
};

////////////////////////////////////////////////////////////
struct FireParticle final : Particle
{
    void draw(sf::RenderTarget& rt) override
    {
        drawParticleImpl(rt, position, {scale, scale}, rotation, txrFire, opacity);
    }
};

////////////////////////////////////////////////////////////
struct SmokeEmitter final : Emitter
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
struct FireEmitter final : Emitter
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
struct Rocket final : Entity
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

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
#endif

////////////////////////////////////////////////////////////
namespace Shared
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

////////////////////////////////////////////////////////////
template <typename TParticle>
[[nodiscard, gnu::always_inline]] inline TParticle makeAoSSmokeParticle(const sf::Vec2f position)
{
    return {
        .position     = position,
        .velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f,
        .acceleration = {0.f, -0.011f},

        .scale    = rng.getF(0.0025f, 0.0035f),
        .opacity  = rng.getF(0.05f, 0.25f),
        .rotation = rng.getF(0.f, 6.28f),

        .scaleRate       = rng.getF(0.001f, 0.003f) * 2.75f,
        .opacityChange   = -rng.getF(0.001f, 0.002f) * 3.25f,
        .angularVelocity = rng.getF(-0.02f, 0.02f),
    };
}


////////////////////////////////////////////////////////////
template <typename TParticle>
[[nodiscard, gnu::always_inline]] inline TParticle makeAoSFireParticle(const sf::Vec2f position)
{
    return {
        .position     = position,
        .velocity     = rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}),
        .acceleration = {0.f, 0.07f},

        .scale    = rng.getF(0.5f, 0.7f) * 0.085f,
        .opacity  = rng.getF(0.2f, 0.4f) * 0.85f,
        .rotation = rng.getF(0.f, 6.28f),

        .scaleRate       = -rng.getF(0.001f, 0.003f) * 0.25f,
        .opacityChange   = -0.001f,
        .angularVelocity = rng.getF(-0.002f, 0.002f),
    };
}

#pragma GCC diagnostic pop

}; // namespace Shared


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
struct Particle // NOLINT(cppcoreguidelines-pro-type-member-init)
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
struct Rocket // NOLINT(cppcoreguidelines-pro-type-member-init)
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
            .position     = rocket.position,
            .velocity     = {},
            .acceleration = {},
            .spawnTimer   = 0.f,
            .spawnRate    = 2.5f,
            .type         = ParticleType::Smoke,
        });

        rocket.fireEmitterIdx = addEmitter({
            .position     = rocket.position,
            .velocity     = {},
            .acceleration = {},
            .spawnTimer   = 0.f,
            .spawnRate    = 1.25f,
            .type         = ParticleType::Fire,
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
                    particles.pushBack(Shared::makeAoSSmokeParticle<Particle>(e->position));
                else if (e->type == ParticleType::Fire)
                    particles.pushBack(Shared::makeAoSFireParticle<Particle>(e->position));

                particles.back().type = e->type;
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
        const auto drawParticlesInstanced =
            [&](const auto& instanceBuffer, const sf::base::SizeT vboIndexOffset, const sf::FloatRect& txr)
        {
            auto setupSpriteInstanceAttribs = [&](sf::RenderTarget::InstanceAttributeBinder& binder)
            {
                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset]);
                binder.uploadContiguousData(instanceBuffer.size(), instanceBuffer.data());

                constexpr auto stride = sizeof(ParticleInstanceData);

                binder.setup(3, 2, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, position));
                binder.setup(4, 1, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, scale));
                binder.setup(5, 1, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, rotation));
                binder.setup(6, 1, sf::GlDataType::Float, true, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, opacity));
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.drawInstancedIndexedVertices(makeInstancedDrawSettings(instanceBuffer.size()), setupSpriteInstanceAttribs);
        };

        const auto nParticles = particles.size();

        instanceRenderingDataBuffer[0].clear();
        instanceRenderingDataBuffer[0].reserve(nParticles);

        instanceRenderingDataBuffer[1].clear();
        instanceRenderingDataBuffer[1].reserve(nParticles);

        for (const Particle& p : particles)
            (p.type == ParticleType::Smoke ? instanceRenderingDataBuffer[0] : instanceRenderingDataBuffer[1])
                .emplaceBack(p.position, p.scale, p.rotation, p.opacity);

        drawParticlesInstanced(instanceRenderingDataBuffer[0], 0, txrSmoke);
        drawParticlesInstanced(instanceRenderingDataBuffer[1], 1, txrFire);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
    }
};

} // namespace AOS


////////////////////////////////////////////////////////////
namespace Shared
{
////////////////////////////////////////////////////////////
template <typename TEmitter>
struct AddU16EmitterMixin
{
    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::U16 addEmitter(sf::base::Vector<sf::base::Optional<TEmitter>>& emitters, const TEmitter& emitter)
    {
        for (sf::base::SizeT i = 0u; i < emitters.size(); ++i)
            if (!emitters[i].hasValue())
            {
                emitters[i].emplace(emitter);
                return static_cast<sf::base::U16>(i);
            }

        emitters.emplaceBack(emitter);
        return static_cast<sf::base::U16>(emitters.size() - 1);
    }
};

////////////////////////////////////////////////////////////
template <typename TRocket>
struct AddRocketMixin
{
    ////////////////////////////////////////////////////////////
    TRocket& addRocket(this auto&& self, const TRocket& r)
    {
        TRocket& rocket = self.rockets.emplaceBack(r);

        rocket.smokeEmitterIdx = self.addEmitter(self.smokeEmitters,
                                                 {
                                                     .position     = rocket.position,
                                                     .velocity     = {},
                                                     .acceleration = {},
                                                     .spawnTimer   = 0.f,
                                                     .spawnRate    = 2.5f,
                                                 });

        rocket.fireEmitterIdx = self.addEmitter(self.fireEmitters,
                                                {
                                                    .position     = rocket.position,
                                                    .velocity     = {},
                                                    .acceleration = {},
                                                    .spawnTimer   = 0.f,
                                                    .spawnRate    = 1.25f,
                                                });

        return rocket;
    }
};

} // namespace Shared


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
struct Particle // NOLINT(cppcoreguidelines-pro-type-member-init)
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
struct Rocket // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::U16 smokeEmitterIdx;
    sf::base::U16 fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World : Shared::AddU16EmitterMixin<Emitter>, Shared::AddRocketMixin<Rocket>
{
    sf::base::Vector<sf::base::Optional<Emitter>> smokeEmitters;
    sf::base::Vector<sf::base::Optional<Emitter>> fireEmitters;
    sf::base::Vector<Particle>                    smokeParticles;
    sf::base::Vector<Particle>                    fireParticles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        const auto updateParticle = [&](Particle& p)
        {
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;
            p.scale += p.scaleRate * dt;
            p.opacity += p.opacityChange * dt;
            p.rotation += p.angularVelocity * dt;
        };

        const auto updateEmitter = [&](sf::base::Optional<Emitter>& e, auto&& fSpawn)
        {
            if (!e.hasValue())
                return;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                fSpawn();
        };

        for (Particle& p : smokeParticles)
            updateParticle(p);

        for (Particle& p : fireParticles)
            updateParticle(p);

        for (sf::base::Optional<Emitter>& e : smokeEmitters)
            updateEmitter(e, [&] { smokeParticles.pushBack(Shared::makeAoSSmokeParticle<Particle>(e->position)); });

        for (sf::base::Optional<Emitter>& e : fireEmitters)
            updateEmitter(e, [&] { fireParticles.pushBack(Shared::makeAoSFireParticle<Particle>(e->position)); });

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

            instanceRenderingDataBuffer[0].clear();
            instanceRenderingDataBuffer[0].reserve(nParticles);

            for (sf::base::SizeT i = 0u; i < nParticles; ++i)
                instanceRenderingDataBuffer[0]
                    .emplaceBack(particles[i].position, particles[i].scale, particles[i].rotation, particles[i].opacity);

            auto setupSpriteInstanceAttribs = [&](sf::RenderTarget::InstanceAttributeBinder& binder)
            {
                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset]);
                binder.uploadContiguousData(nParticles, instanceRenderingDataBuffer[0].data());

                constexpr auto stride = sizeof(ParticleInstanceData);

                binder.setup(3, 2, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, position));
                binder.setup(4, 1, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, scale));
                binder.setup(5, 1, sf::GlDataType::Float, false, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, rotation));
                binder.setup(6, 1, sf::GlDataType::Float, true, stride, SFML_BASE_OFFSETOF(ParticleInstanceData, opacity));
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.drawInstancedIndexedVertices(makeInstancedDrawSettings(nParticles), setupSpriteInstanceAttribs);
        };

        drawParticlesInstanced(0, txrSmoke, smokeParticles);
        drawParticlesInstanced(1, txrFire, fireParticles);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
    }
};

} // namespace AOSImproved


////////////////////////////////////////////////////////////
namespace SOAManual
{
////////////////////////////////////////////////////////////
struct ParticleSoA
{
    sf::base::Vector<sf::Vec2f> positions;
    sf::base::Vector<sf::Vec2f> velocities;
    sf::base::Vector<sf::Vec2f> accelerations;

    sf::base::Vector<float> scales;
    sf::base::Vector<float> opacities;
    sf::base::Vector<float> rotations;

    sf::base::Vector<float> scaleRates;
    sf::base::Vector<float> opacityChanges;
    sf::base::Vector<float> angularVelocities;

    ////////////////////////////////////////////////////////////
    void forEachVector(auto&& f)
    {
        f(positions);
        f(velocities);
        f(accelerations);

        f(scales);
        f(opacities);
        f(rotations);

        f(scaleRates);
        f(opacityChanges);
        f(angularVelocities);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::base::SizeT getSize() const
    {
        return positions.size();
    }
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
struct Rocket // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::U16 smokeEmitterIdx;
    sf::base::U16 fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World : Shared::AddU16EmitterMixin<Emitter>, Shared::AddRocketMixin<Rocket>
{
    sf::base::Vector<sf::base::Optional<Emitter>> smokeEmitters;
    sf::base::Vector<sf::base::Optional<Emitter>> fireEmitters;
    ParticleSoA                                   smokeParticles;
    ParticleSoA                                   fireParticles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        const auto updateParticles = [&](auto& soa)
        {
            const auto nParticles = soa.getSize();

            for (sf::base::SizeT i = 0u; i < nParticles; ++i)
            {
                soa.velocities[i] += soa.accelerations[i] * dt;
                soa.positions[i] += soa.velocities[i] * dt;
                soa.scales[i] += soa.scaleRates[i] * dt;
                soa.opacities[i] += soa.opacityChanges[i] * dt;
                soa.rotations[i] += soa.angularVelocities[i] * dt;
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
            {
                smokeParticles.positions.pushBack(e->position);
                smokeParticles.velocities.pushBack(rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f);
                smokeParticles.accelerations.pushBack({0.f, -0.011f});
                smokeParticles.scales.pushBack(rng.getF(0.0025f, 0.0035f));
                smokeParticles.opacities.pushBack(rng.getF(0.05f, 0.25f));
                smokeParticles.rotations.pushBack(rng.getF(0.f, 6.28f));
                smokeParticles.scaleRates.pushBack(rng.getF(0.001f, 0.003f) * 2.75f);
                smokeParticles.opacityChanges.pushBack(-rng.getF(0.001f, 0.002f) * 3.25f);
                smokeParticles.angularVelocities.pushBack(rng.getF(-0.02f, 0.02f));
            }
        }

        for (sf::base::Optional<Emitter>& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
            {
                fireParticles.positions.pushBack(e->position);
                fireParticles.velocities.pushBack(rng.getVec2f({-0.3f, -0.8f}, {0.3f, -0.2f}));
                fireParticles.accelerations.pushBack({0.f, 0.07f});
                fireParticles.scales.pushBack(rng.getF(0.5f, 0.7f) * 0.085f);
                fireParticles.opacities.pushBack(rng.getF(0.2f, 0.4f) * 0.85f);
                fireParticles.rotations.pushBack(rng.getF(0.f, 6.28f));
                fireParticles.scaleRates.pushBack(-rng.getF(0.001f, 0.003f) * 0.25f);
                fireParticles.opacityChanges.pushBack(-0.001f);
                fireParticles.angularVelocities.pushBack(rng.getF(-0.002f, 0.002f));
            }
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
        const auto soaEraseIf = [&](ParticleSoA& soa, auto&& predicate)
        {
            sf::base::SizeT currentSize = soa.positions.size();

            for (sf::base::SizeT i = currentSize; i-- > 0u;)
            {
                if (!predicate(soa, i))
                    continue;

                --currentSize;
                soa.forEachVector([&](auto& vec) { vec[i] = SFML_BASE_MOVE(vec[currentSize]); });
            }

            soa.forEachVector([&](auto& vec) { vec.resize(currentSize); });
        };

        soaEraseIf(smokeParticles,
                   [](const ParticleSoA& soa, const sf::base::SizeT i) { return soa.opacities[i] <= 0.f; });
        soaEraseIf(fireParticles, [](const ParticleSoA& soa, const sf::base::SizeT i) { return soa.opacities[i] <= 0.f; });

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
            const auto nParticles = particles.positions.size();

            auto setupSpriteInstanceAttribs = [&](sf::RenderTarget::InstanceAttributeBinder& binder)
            {
                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 0]);
                binder.uploadContiguousData(nParticles, particles.positions.data());
                binder.setup(3, 2, sf::GlDataType::Float, false, sizeof(sf::Vec2f), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 1]);
                binder.uploadContiguousData(nParticles, particles.scales.data());
                binder.setup(4, 1, sf::GlDataType::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 2]);
                binder.uploadContiguousData(nParticles, particles.rotations.data());
                binder.setup(5, 1, sf::GlDataType::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 3]);
                binder.uploadContiguousData(nParticles, particles.opacities.data());
                binder.setup(6, 1, sf::GlDataType::Float, true, sizeof(float), 0u);
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.drawInstancedIndexedVertices(makeInstancedDrawSettings(nParticles), setupSpriteInstanceAttribs);
        };

        drawParticlesInstanced(0, txrSmoke, smokeParticles);
        drawParticlesInstanced(4, txrFire, fireParticles);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
    }
};

} // namespace SOAManual

////////////////////////////////////////////////////////////
namespace SOAMeta
{
////////////////////////////////////////////////////////////
struct Particle // NOLINT(cppcoreguidelines-pro-type-member-init)
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
using ParticleSoA = SoAFor<Particle>;

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
struct Rocket // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    sf::base::U16 smokeEmitterIdx;
    sf::base::U16 fireEmitterIdx;
};

////////////////////////////////////////////////////////////
struct World : Shared::AddU16EmitterMixin<Emitter>, Shared::AddRocketMixin<Rocket>
{
    sf::base::Vector<sf::base::Optional<Emitter>> smokeEmitters;
    sf::base::Vector<sf::base::Optional<Emitter>> fireEmitters;
    ParticleSoA                                   smokeParticles;
    ParticleSoA                                   fireParticles;
    sf::base::Vector<Rocket>                      rockets;

    ////////////////////////////////////////////////////////////
    void update(const float dt)
    {
        auto updateParticles = [&](auto& soa)
        {
            soa.withAll(
                [&](sf::Vec2f&       position,
                    sf::Vec2f&       velocity,
                    const sf::Vec2f& acceleration,
                    float&           scale,
                    float&           opacity,
                    float&           rotation,
                    const float&     scaleRate,
                    const float&     opacityChange,
                    const float&     angularVelocity)
            {
                velocity += acceleration * dt;
                position += velocity * dt;
                scale += scaleRate * dt;
                opacity += opacityChange * dt;
                rotation += angularVelocity * dt;
            });
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
                smokeParticles.pushBack(Shared::makeAoSSmokeParticle<Particle>(e->position));
        }

        for (sf::base::Optional<Emitter>& e : fireEmitters)
        {
            if (!e.hasValue())
                continue;

            e->position += e->velocity * dt;
            e->velocity += e->acceleration * dt;
            e->spawnTimer += e->spawnRate * dt;

            for (; e->spawnTimer >= 1.f; e->spawnTimer -= 1.f)
                fireParticles.pushBack(Shared::makeAoSFireParticle<Particle>(e->position));
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
        smokeParticles.eraseIfBySwapping<&Particle::opacity>([](const float opacity) { return opacity <= 0.f; });
        fireParticles.eraseIfBySwapping<&Particle::opacity>([](const float opacity) { return opacity <= 0.f; });

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
                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 0]);
                binder.uploadContiguousData(nParticles, particles.template get<&Particle::position>().data());
                binder.setup(3, 2, sf::GlDataType::Float, false, sizeof(sf::Vec2f), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 1]);
                binder.uploadContiguousData(nParticles, particles.template get<&Particle::scale>().data());
                binder.setup(4, 1, sf::GlDataType::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 2]);
                binder.uploadContiguousData(nParticles, particles.template get<&Particle::rotation>().data());
                binder.setup(5, 1, sf::GlDataType::Float, false, sizeof(float), 0u);

                binder.bindVBO(*instanceRenderingVBOs[vboIndexOffset + 3]);
                binder.uploadContiguousData(nParticles, particles.template get<&Particle::opacity>().data());
                binder.setup(6, 1, sf::GlDataType::Float, true, sizeof(float), 0u);
            };

            instanceRenderingShader->setUniform(*instanceRenderingULTextureRect,
                                                sf::Glsl::Vec4{txr.position.x, txr.position.y, txr.size.x, txr.size.y});

            rt.drawInstancedIndexedVertices(makeInstancedDrawSettings(nParticles), setupSpriteInstanceAttribs);
        };

        drawParticlesInstanced(0, txrSmoke, smokeParticles);
        drawParticlesInstanced(4, txrFire, fireParticles);

        for (const auto& r : rockets)
            drawRocketImpl(rt, r.position);
    }
};

} // namespace SOAMeta

////////////////////////////////////////////////////////////
enum class Mode
{
    OOP,
    AOS,
    AOSImproved,
    SOAManual,
    SOAMeta,
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
    constexpr sf::Vec2f resolution{1680.f, 1050.f};

    auto window = makeDPIScaledRenderWindow({
        .size            = resolution.toVec2u(),
        .title           = "Rockets",
        .fullscreen      = false,
        .resizable       = false,
        .closable        = false,
        .hasTitlebar     = false,
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
    // Set up presentation textures
    sf::Texture txBanners[4] = {
        sf::Texture::loadFromFile("resources/banner0.png").value(),
        sf::Texture::loadFromFile("resources/banner1.png").value(),
        sf::Texture::loadFromFile("resources/banner2.png").value(),
        sf::Texture::loadFromFile("resources/banner3.png").value(),
    };

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
    bool  drawUI          = false;
    float simulationSpeed = 0.1f;
    float rocketSpawnRate = 0.f;
    float zoom            = 3.f;
    float imguiMult       = 1.f;

    //
    //
    // Presentation state
    float bannerAlpha[4]       = {0.f, 0.f, 0.f, 0.f};
    float bannerTargetAlpha[4] = {0.f, 0.f, 0.f, 0.f};

    //
    //
    // Simulation state
    float              rocketSpawnTimer = 0.f;
    OOP::World         oopWorld;
    AOS::World         aosWorld;
    AOSImproved::World aosImprovedWorld;
    SOAManual::World   soaManualWorld;
    SOAMeta::World     soaMetaWorld;

    oopWorld.entities.reserve(1'250'000);

    aosWorld.rockets.reserve(2500);
    aosWorld.emitters.reserve(5000);
    aosWorld.particles.reserve(1'000'000);

    aosImprovedWorld.rockets.reserve(2500);
    aosImprovedWorld.smokeEmitters.reserve(2500);
    aosImprovedWorld.fireEmitters.reserve(2500);
    aosImprovedWorld.smokeParticles.reserve(750'000);
    aosImprovedWorld.fireParticles.reserve(750'000);

    soaManualWorld.rockets.reserve(2500);
    soaManualWorld.smokeEmitters.reserve(2500);
    soaManualWorld.fireEmitters.reserve(2500);
    soaManualWorld.smokeParticles.forEachVector([](auto& vec) { vec.reserve(750'000); });
    soaManualWorld.fireParticles.forEachVector([](auto& vec) { vec.reserve(750'000); });

    soaMetaWorld.rockets.reserve(2500);
    soaMetaWorld.smokeEmitters.reserve(2500);
    soaMetaWorld.fireEmitters.reserve(2500);
    soaMetaWorld.smokeParticles.reserve(750'000);
    soaMetaWorld.fireParticles.reserve(750'000);

    //
    //
    // Set up clock and time sampling
    sf::Clock clock;
    sf::Clock fpsClock;

    Sampler samplesUpdateMs(/* capacity */ 32u);
    Sampler samplesDrawMs(/* capacity */ 32u);
    Sampler samplesDisplayMs(/* capacity */ 32u);
    Sampler samplesFPS(/* capacity */ 32u);

    //
    //
    // Helper functions
    const auto makeFullscreen = [&]
    {
        window.setResizable(false);
        window.setHasTitlebar(false);

        window.setSize(sf::VideoModeUtils::getDesktopMode().size);
        window.setPosition({0, 0});
    };

    const auto makeWindowed = [&]
    {
        window.setResizable(true);
        window.setHasTitlebar(true);

        window.setSize(sf::VideoModeUtils::getDesktopMode().size / 2.f);
        window.setCenter(sf::VideoModeUtils::getDesktopMode().size / 2.f);
    };

    //
    //
    // Startup as windowed mode
    makeWindowed();

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

                if (handleAspectRatioAwareResize(*event, resolution, window))
                    continue;

                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (eKeyPressed->code == sf::Keyboard::Key::Num1)
                    {
                        mode            = Mode::OOP;
                        drawStep        = true;
                        drawUI          = false;
                        simulationSpeed = 0.1f;
                        rocketSpawnRate = 0.f;
                        zoom            = 3.f;

                        oopWorld = {};

                        for (int k = 0; k < 64; ++k)
                        {
                            {
                                auto& r        = oopWorld.addEntity<OOP::Rocket>();
                                r.position     = {-256.f * static_cast<float>(k) + -16.f, resolution.y / 2.f};
                                r.velocity     = {1.0f, 0.f};
                                r.acceleration = {0.03f, 0.f};
                                r.init();
                            }

                            for (int i = 1; i <= 5; ++i)
                            {
                                auto& r0    = oopWorld.addEntity<OOP::Rocket>();
                                r0.position = {-256.f * static_cast<float>(k) - 16.f + -48.f * static_cast<float>(i),
                                               resolution.y / 2.f - 32.f * static_cast<float>(i)};
                                r0.velocity = {1.0f, 0.f};
                                r0.acceleration = {0.03f, 0.f};
                                r0.init();

                                auto& r1    = oopWorld.addEntity<OOP::Rocket>();
                                r1.position = {-256.f * static_cast<float>(k) - 16.f + -48.f * static_cast<float>(i),
                                               resolution.y / 2.f + 32.f * static_cast<float>(i)};
                                r1.velocity = {1.0f, 0.f};
                                r1.acceleration = {0.03f, 0.f};
                                r1.init();
                            }
                        }
                    }
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num2)
                        bannerTargetAlpha[0] = 1.f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num3)
                        bannerTargetAlpha[1] = 1.f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num4)
                        bannerTargetAlpha[2] = 1.f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num5)
                        bannerTargetAlpha[3] = 1.f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num6)
                    {
                        for (float& ta : bannerTargetAlpha)
                            ta = 0.f;

                        drawUI = true;
                    }
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num8)
                        imguiMult -= 0.25f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::Num9)
                        imguiMult += 0.25f;
                    else if (eKeyPressed->code == sf::Keyboard::Key::F)
                        makeFullscreen();
                    else if (eKeyPressed->code == sf::Keyboard::Key::W)
                        makeWindowed();
                }
            }
        }
        // ---

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            rocketSpawnTimer += rocketSpawnRate * simulationSpeed;

            const auto updateNonOOPWorld = [&](auto& world)
            {
                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    world.addRocket({
                        .position     = rng.getVec2f({-500.f, 0.f}, {-100.f, resolution.y}),
                        .velocity     = {},
                        .acceleration = {rng.getF(0.01f, 0.025f), 0.f},
                    });
                }

                world.update(simulationSpeed);
                world.cleanup();
            };

            if (mode == Mode::OOP)
            {
                for (; rocketSpawnTimer >= 1.f; rocketSpawnTimer -= 1.f)
                {
                    auto& rocket = oopWorld.addEntity<OOP::Rocket>();

                    rocket.position     = rng.getVec2f({-500.f, 0.f}, {-100.f, resolution.y});
                    rocket.velocity     = {};
                    rocket.acceleration = {rng.getF(0.01f, 0.025f), 0.f};

                    rocket.init();
                }

                oopWorld.update(simulationSpeed);
                oopWorld.cleanup();
            }
            else if (mode == Mode::AOS)
                updateNonOOPWorld(aosWorld);
            else if (mode == Mode::AOSImproved)
                updateNonOOPWorld(aosImprovedWorld);
            else if (mode == Mode::SOAManual)
                updateNonOOPWorld(soaManualWorld);
            else if (mode == Mode::SOAMeta)
                updateNonOOPWorld(soaMetaWorld);

            const auto moveFloatTowards = [](float& value, const float target, const float delta)
            {
                if (value < target)
                    value = sf::base::min(target, value + delta);
                else if (value > target)
                    value = sf::base::max(target, value - delta);
            };

            for (int i = 0; i < 4; ++i)
                moveFloatTowards(bannerAlpha[i], bannerTargetAlpha[i], 0.025f);
        }
        samplesUpdateMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

#pragma GCC diagnostic pop

        ////////////////////////////////////////////////////////////
        // ImGui step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();

        if (drawUI)
        {
            const auto clearSamples = [&]
            {
                samplesUpdateMs.clear();
                samplesDrawMs.clear();
                samplesDisplayMs.clear();
                samplesFPS.clear();
            };

            const auto setFontScale = [&](const float x) { ImGui::SetWindowFontScale(x * imguiMult); };

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
                                 ImVec2{128.f * imguiMult, 32.f * imguiMult});
            };

            ImGui::SetNextWindowSize(ImVec2{440.f * imguiMult, 470.f * imguiMult});
            ImGui::SetNextWindowPos({window.getRight() - 440.f * imguiMult - 24.f * imguiMult, 24.f * imguiMult});

            ImGui::PushFont(fontImGuiGeistMono);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f * imguiMult); // Set corner radius

            ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            setFontScale(1.25f);

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
            setFontScale(1.f);

            const auto printDODEntityCount = [](const auto& world)
            {
                const auto getSize = [](const auto& container)
                {
                    if constexpr (requires { container.size(); })
                    {
                        return container.size();
                    }
                    else
                    {
                        return container.getSize();
                    }
                };

                ImGui::Text("Number of entities: %zu",
                            getSize(world.rockets) + getSize(world.smokeEmitters) + getSize(world.smokeParticles) +
                                getSize(world.fireEmitters) + getSize(world.fireParticles));
            };

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
                printDODEntityCount(aosImprovedWorld);
            else if (mode == Mode::SOAManual)
                printDODEntityCount(soaManualWorld);
            else if (mode == Mode::SOAMeta)
                printDODEntityCount(soaMetaWorld);

            ImGui::Separator();
            setFontScale(0.75f);
            ImGui::Checkbox("Enable rendering", &drawStep);

            ImGui::Separator();
            setFontScale(0.75f);
            if (ImGui::Combo("##infoMode",
                             reinterpret_cast<int*>(&mode),
                             "OOP (Heap-allocated entities)\0AoS (Very branchy)\0AoS (Improved)\0SoA (Manual)\0SoA "
                             "(Meta)\0"))
            {
                clearSamples();
            }

            ImGui::Separator();
            setFontScale(0.75f);

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
            setFontScale(0.75f);

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
            setFontScale(0.75f);

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

            const auto prevView = window.getView();

            window.setView({.center   = {resolution.x / (2.f * zoom), resolution.y / 2.f},
                            .size     = prevView.size / zoom,
                            .viewport = prevView.viewport});

            if (drawStep)
            {
                if (mode == Mode::OOP)
                    oopWorld.draw(window);
                else if (mode == Mode::AOS)
                    aosWorld.draw(window);
                else if (mode == Mode::AOSImproved)
                    aosImprovedWorld.draw(window);
                else if (mode == Mode::SOAManual)
                    soaManualWorld.draw(window);
                else if (mode == Mode::SOAMeta)
                    soaMetaWorld.draw(window);
            }

            window.setView(prevView);

            for (int i = 0; i < 4; ++i)
            {
                constexpr float offset = 53.f;

                float top = offset;

                for (int j = 0; j < i; ++j)
                    top += txBanners[j].getRect().size.y + offset;

                window.draw(txBanners[i],
                            {
                                .position = {(resolution.x - static_cast<float>(txBanners[0].getSize().x)) / 2.f, top},
                                .textureRect = txBanners[i].getRect(),
                                .color = sf::Color::whiteMask(static_cast<sf::base::U8>(bannerAlpha[i] * 255.f * 0.85f)),
                            });
            }
        }
        samplesDrawMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        // ---
        clock.restart();
        {
            if (drawUI)
                imGuiContext.render(window);

            window.display();
        }
        samplesDisplayMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        samplesFPS.record(1.f / fpsClock.getElapsedTime().asSeconds());
    }
}

#ifdef __clang__
    #pragma clang diagnostic pop
#endif
