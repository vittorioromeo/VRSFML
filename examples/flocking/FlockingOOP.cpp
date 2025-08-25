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
struct World;

////////////////////////////////////////////////////////////
struct Entity
{
    bool alive = true;

    virtual ~Entity() = default;

    virtual void update(World&, float)
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
            entities[i]->update(*this, dt);
    }

    void draw(sf::RenderTarget& rt)
    {
        for (const auto& entity : entities)
            entity->draw(rt);
    }
};

////////////////////////////////////////////////////////////
struct Emitter : Entity
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    float life;
    float lifeChange;

    void update(World&, float dt) override
    {
        life += lifeChange * dt;

        if (life <= 0.f)
            alive = false;
    }

    void draw(sf::RenderTarget&) override
    {
    }
};

////////////////////////////////////////////////////////////
struct Particle : Entity
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

    void update(World&, float dt) override
    {
        velocity += acceleration * dt;
        position += velocity * dt;

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
    void update(World& world, float dt) override
    {
        Emitter::update(world, dt);

        auto p = sf::base::makeUnique<SmokeParticle>();

        p->position     = position;
        p->velocity     = rng.getVec2f({-0.2f, -0.2f}, {0.2f, 0.2f}) * 0.5f;
        p->acceleration = {0.f, -0.002f};

        p->life     = 1.f;
        p->scale    = rng.getF(0.005f, 0.02f);
        p->opacity  = rng.getF(0.05f, 0.25f);
        p->rotation = rng.getF(0.f, 6.28f);

        p->lifeChange     = -0.001f;
        p->scaleChange    = rng.getF(0.001f, 0.003f) * 0.5f;
        p->opacityChange  = -rng.getF(0.001f, 0.002f) * 0.9f;
        p->rotationChange = rng.getF(-0.001f, 0.001f);

        world.entities.emplaceBack(SFML_BASE_MOVE(p));
    }
};

////////////////////////////////////////////////////////////
struct FireEmitter : Emitter
{
    void update(World& world, float dt) override
    {
        Emitter::update(world, dt);

        auto p = sf::base::makeUnique<FireParticle>();

        p->position     = position;
        p->velocity     = rng.getVec2f({-0.3f, -0.2f}, {0.3f, 0.f});
        p->acceleration = {0.f, 0.006f};

        p->life     = 1.f;
        p->scale    = rng.getF(0.01f, 0.03f) * 0.75f;
        p->opacity  = rng.getF(0.2f, 0.4f) * 0.85f;
        p->rotation = 0.f;

        p->lifeChange     = -0.003f;
        p->scaleChange    = rng.getF(0.001f, 0.003f) * 0.25f;
        p->opacityChange  = -0.0015f;
        p->rotationChange = rng.getF(-0.002f, 0.002f);

        world.entities.emplaceBack(SFML_BASE_MOVE(p));
    }
};

////////////////////////////////////////////////////////////
struct Rocket : Entity
{
    sf::Vec2f position;
    sf::Vec2f velocity;
    sf::Vec2f acceleration;

    SmokeEmitter* smokeEmitter;
    FireEmitter*  fireEmitter;

    bool init = false;

    void update(World& world, float dt) override
    {
        if (!init)
        {
            init = true;

            auto se          = sf::base::makeUnique<SmokeEmitter>();
            se->position     = position;
            se->velocity     = {};
            se->acceleration = {};
            se->life         = 100.f;
            se->lifeChange   = -0.01f;
            smokeEmitter     = se.get();
            world.entities.emplaceBack(SFML_BASE_MOVE(se));

            auto fe          = sf::base::makeUnique<FireEmitter>();
            fe->position     = position;
            fe->velocity     = {};
            fe->acceleration = {};
            fe->life         = 100.f;
            fe->lifeChange   = -0.01f;
            fireEmitter      = fe.get();
            world.entities.emplaceBack(SFML_BASE_MOVE(fe));
        }

        velocity += acceleration * dt;
        position += velocity * dt;

        smokeEmitter->position = position - sf::Vec2f{12.f, 0.f};
        fireEmitter->position  = position - sf::Vec2f{12.f, 0.f};

        if (position.x > 2000.f)
        {
            alive               = false;
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
        .size      = windowSize.toVec2u(),
        .title     = "Vittorio's SFML fork: particles example",
        .resizable = false,
        .vsync     = false,
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
    // OOP Particles
    World       world;
    std::size_t nTargetRockets = 1000;

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
            std::size_t nRockets = 0;
            for (const auto& entity : world.entities)
            {
                if (dynamic_cast<Rocket*>(entity.get()))
                    ++nRockets;
            }

            for (std::size_t i = 0; i < nTargetRockets - nRockets; ++i)
            {
                auto emitter = sf::base::makeUnique<Rocket>();

                emitter->position     = rng.getVec2f({-1000.f, 0.f}, {-100.f, windowSize.y});
                emitter->velocity     = {};
                emitter->acceleration = {rng.getF(0.01f, 0.05f), 0.f};

                world.entities.emplaceBack(SFML_BASE_MOVE(emitter));
            }

            world.update(1.f);
            world.cleanup();
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

            ImGui::Text("Number of entities: %zu", world.entities.size());

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
            world.draw(window);
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
