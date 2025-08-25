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
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/InterferenceSize.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ThreadPool.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <latch>
#include <string>


namespace
{
////////////////////////////////////////////////////////////
RNGFast rng;

////////////////////////////////////////////////////////////
sf::base::Optional<sf::Texture> txSmoke;

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
/*
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
*/

////////////////////////////////////////////////////////////
/*
void drawSmokeParticle(const Particle& p, sf::RenderTarget& rt)
{
    rt.draw(
        sf::Sprite{
            .position    = p.position,
            .scale       = {p.scale, p.scale},
            .origin      = txSmoke->getSize() / 2.f,
            .rotation    = sf::radians(p.rotation),
            .textureRect = txSmoke->getRect(),
            .color       = sf::Color::whiteMask(static_cast<sf::base::U8>((p.opacity / 1.f) * 255.f)),
        },
        sf::RenderStates{.texture = txSmoke.asPtr()});
}
*/

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

    txSmoke = sf::Texture::loadFromFile("resources/pSmoke.png");

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
    sf::TextureAtlas textureAtlas{sf::Texture::create({512u, 512u}, {.smooth = true}).value()};

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
    };

    //
    //
    // DOD Particles
    sf::base::Vector<Emitter> smokeEmitters;

    using ParticleSoA = SoAFor<sf::Vec2f, // position
                               sf::Vec2f, // velocity
                               sf::Vec2f, // acceleration

                               float, // life
                               float, // scale
                               float, // opacity
                               float, // rotation

                               float,  // lifeChange
                               float,  // scaleChange
                               float,  // opacityChange
                               float>; // rotationChange

    ParticleSoA smokeParticles;
    // sf::base::Vector<Particle> smokeParticles;

    for (int i = 0; i < 1000; ++i)
    {
        auto& emitter = smokeEmitters.emplaceBack();

        emitter.position     = rng.getVec2f({0.f, 0.f}, windowSize);
        emitter.velocity     = {};
        emitter.acceleration = {};
        emitter.life         = 100.f;
        emitter.lifeChange   = -0.01f;
    }

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
            const float dt = 1.f;

            for (auto& emitter : smokeEmitters)
            {
                emitter.life += emitter.lifeChange * dt;

                smokeParticles.pushBack(

                    emitter.position,
                    rng.getVec2f({-0.1f, -0.1f}, {0.1f, 0.1f}),
                    sf::Vec2f{0.f, -0.001f},

                    1.f,
                    rng.getF(0.01f, 0.02f),
                    0.5f,
                    0.f,

                    -0.001f,
                    0.002f,
                    -0.001f,
                    rng.getF(-0.001f, 0.001f)

                );
            }

            smokeParticles.with<1, 2>([](sf::Vec2f& velocity, const sf::Vec2f& acc) { velocity += acc; });
            smokeParticles.with<0, 1>([](sf::Vec2f& position, sf::Vec2f& velocity) { position += velocity; });

            smokeParticles.with<3, 7>([](float& life, const float& lifeChange) { life += lifeChange; });
            smokeParticles.with<4, 8>([](float& scale, const float& scaleChange)
            {
                scale += scaleChange;
                if (scale <= 0.f)
                    scale = 0.f;
            });
            smokeParticles.with<5, 9>([](float& opacity, const float& opacityChange)
            {
                opacity += opacityChange;
                if (opacity <= 0.f)
                    opacity = 0.f;
            });
            smokeParticles.with<6, 10>([](float& rotation, const float& rotationChange) { rotation += rotationChange; });

            sf::base::vectorEraseIf(smokeEmitters, [](const Emitter& e) { return e.life <= 0.f; });
            smokeParticles.eraseIfBySwapping<3>([](const float& life) { return life <= 0.f; });
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

            ImGui::Text("Number of entities: %zu", smokeEmitters.size() + smokeParticles.getSize());

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
            // world.draw(window);
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
