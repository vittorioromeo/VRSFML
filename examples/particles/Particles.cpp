#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/SoA.hpp"     // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

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
[[gnu::always_inline]] void updateParticle(
    sf::Vec2f&      position,
    sf::Vec2f&      velocity,
    const sf::Vec2f acceleration,

    float&      scale,
    const float scaleGrowth,

    float&      opacity,
    const float opacityGrowth,

    float&      rotation,
    const float torque)
{
    velocity += acceleration;
    position += velocity;
    scale += scaleGrowth;
    opacity += opacityGrowth;
    rotation += torque;
}

////////////////////////////////////////////////////////////
[[gnu::always_inline]] void drawParticleImpl(
    const sf::Texture&   texture,
    const sf::FloatRect& txr,
    const sf::Vec2f      origin,
    const sf::Vec2f      position,
    const float          scale,
    const float          rotation,
    auto&&               drawFn)
{
    drawFn(
        sf::Sprite{
            .position    = position,
            .scale       = {scale, scale},
            .origin      = origin,
            .rotation    = sf::radians(rotation),
            .textureRect = txr,
        },
        sf::RenderStates{.texture = &texture});
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up random generator
    RNGFast rng;

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
    // OOP Particles
    struct Entity
    {
        virtual ~Entity() = default;

        virtual void update()
        {
        }

        virtual void draw(const sf::Texture&, sf::RenderTarget&)
        {
        }
    };

    struct ParticleOOP : Entity
    {
        sf::Vec2f position;
        sf::Vec2f velocity;
        sf::Vec2f acceleration;

        float scale;
        float scaleGrowth;

        float opacity;
        float opacityGrowth;

        float rotation;
        float torque;

        ParticleOOP(sf::Vec2f position,
                    sf::Vec2f velocity,
                    sf::Vec2f acceleration,
                    float     scale,
                    float     scaleGrowth,
                    float     opacity,
                    float     opacityGrowth,
                    float     rotation,
                    float     torque) :
            position(position),
            velocity(velocity),
            acceleration(acceleration),
            scale(scale),
            scaleGrowth(scaleGrowth),
            opacity(opacity),
            opacityGrowth(opacityGrowth),
            rotation(rotation),
            torque(torque)
        {
        }

        void update() override
        {
            updateParticle(position, velocity, acceleration, scale, scaleGrowth, opacity, opacityGrowth, rotation, torque);
        }

        void draw(const sf::Texture& texture, sf::RenderTarget& rt) override
        {
            drawParticleImpl(texture,
                             {{0.f, 0.f}, {64.f, 64.f}},
                             {32.f, 32.f},
                             position,
                             scale,
                             rotation,
                             [&](const sf::Sprite& sprite, const sf::RenderStates& states) { rt.draw(sprite, states); });
        }
    };

    sf::base::Vector<sf::base::UniquePtr<Entity>> entities;

    //
    //
    // SoA Particles
    struct ParticleAoS
    {
        sf::Vec2f position;
        sf::Vec2f velocity;
        sf::Vec2f acceleration;

        float scale;
        float scaleGrowth;

        float opacity;
        float opacityGrowth;

        float rotation;
        float torque;
    };

    sf::base::Vector<ParticleAoS> particlesAoS;

    //
    //
    // AoS Particles
    using ParticleSoA = SoAFor<sf::Vec2f, // position
                               sf::Vec2f, // velocity
                               sf::Vec2f, // acceleration

                               float, // scale
                               float, // scaleGrowth

                               float, // opacity
                               float, // opacityGrowth

                               float,  // rotation
                               float>; // torque

    ParticleSoA particlesSoA;

    //
    //
    // Get hardware constants
    const auto     nMaxWorkers   = static_cast<sf::base::U64>(sf::base::ThreadPool::getHardwareWorkerCount());
    constexpr auto cacheLineSize = static_cast<sf::base::SizeT>(sf::base::hardwareDestructiveInterferenceSize);

    //
    //
    // Set up UI elements
    enum class BatchType : int
    {
        Disabled   = 0,
        CPUStorage = 1,
        GPUStorage = 2
    };

    const auto defaultBatchType =
#ifdef SFML_OPENGL_ES
        BatchType::CPUStorage
#else
        BatchType::GPUStorage
#endif
        ;

    auto          batchType                = BatchType::Disabled;
    auto          autobatchType            = defaultBatchType;
    sf::base::U64 autoBatchVertexThreshold = 32'768u;
    bool          multithreadedUpdate      = false;
    bool          multithreadedDraw        = false;
    bool          useOOP                   = false;
    bool          useSoA                   = false;
    bool          unifiedSoAProcessing     = false;
    bool          destroyParticles         = true;
    bool          destroyBySwapping        = true;
    sf::base::U64 nWorkers                 = nMaxWorkers;
    int           numEntities              = 50'000;
    bool          drawStep                 = true;

    //
    //
    // Set up drawable batches
    struct alignas(cacheLineSize) AlignedCPUDrawableBatch : sf::CPUDrawableBatch
    {
        using sf::CPUDrawableBatch::CPUDrawableBatch;
    };

    struct alignas(cacheLineSize) AlignedGPUDrawableBatch : sf::PersistentGPUDrawableBatch
    {
        using sf::PersistentGPUDrawableBatch::PersistentGPUDrawableBatch;
    };

    sf::base::Vector<AlignedCPUDrawableBatch> cpuDrawableBatches(static_cast<sf::base::SizeT>(nMaxWorkers));
    sf::base::Vector<AlignedGPUDrawableBatch> gpuDrawableBatches(static_cast<sf::base::SizeT>(nMaxWorkers));

    //
    //
    // Set up thread pool
    sf::base::ThreadPool pool(nMaxWorkers);

    const auto doInBatches = [&](const sf::base::SizeT nParticlesTotal, auto&& f)
    {
        const sf::base::SizeT particlesPerBatch = nParticlesTotal / nWorkers;

        std::latch latch{static_cast<sf::base::PtrDiffT>(nWorkers)};

        for (sf::base::SizeT i = 0u; i < nWorkers; ++i)
        {
            pool.post([&, i]
            {
                const sf::base::SizeT batchStartIdx = i * particlesPerBatch;
                const sf::base::SizeT batchEndIdx = (i == nWorkers - 1u) ? nParticlesTotal : (i + 1u) * particlesPerBatch;

                f(i, batchStartIdx, batchEndIdx);

                latch.count_down();
            });
        }

        latch.wait();
    };

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
    // Population functions
    const auto pushParticle = [&](auto&& pushFn) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        pushFn(rng.getVec2f({0.f, 0.f}, windowSize),       // position
               rng.getVec2f({-0.5f, -0.5f}, {0.5f, 0.5f}), // velocity
               rng.getVec2f({-0.1f, -0.1f}, {0.1f, 0.1f}), // acceleration
               rng.getF(0.001f, 0.015f),                   // scale
               rng.getF(-0.002f, 0.002f),                  // scaleGrowth
               rng.getF(0.75f, 1.f),                       // opacity
               rng.getF(-0.0015f, -0.0005f),               // opacityGrowth
               rng.getF(0.f, sf::base::tau),               // rotation
               rng.getF(-0.005f, 0.005f)                   // torque
        );
    };

    const auto populateParticlesOOP = [&](const sf::base::SizeT n)
    {
        if (n < entities.size())
        {
            entities.resize(n);
            return;
        }

        entities.reserve(n);

        for (sf::base::SizeT i = entities.size(); i < n; ++i)
            pushParticle([&](auto&&... args) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
            { entities.emplaceBack(sf::base::makeUnique<ParticleOOP>(args...)); });
    };

    const auto populateParticlesAoS = [&](const sf::base::SizeT n)
    {
        if (n < particlesAoS.size())
        {
            particlesAoS.resize(n);
            return;
        }

        particlesAoS.reserve(n);

        for (sf::base::SizeT i = particlesAoS.size(); i < n; ++i)
            pushParticle([&](auto&&... args) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
            { particlesAoS.emplaceBack(args...); });
    };

    const auto populateParticlesSoA = [&](const sf::base::SizeT n)
    {
        if (n < particlesSoA.getSize())
        {
            particlesSoA.resize(n);
            return;
        }

        particlesSoA.reserve(n);

        for (sf::base::SizeT i = particlesSoA.getSize(); i < n; ++i)
            pushParticle([&](auto&&... args) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { particlesSoA.pushBack(args...); });
    };

    const auto populateParticles = [&](const sf::base::SizeT n)
    {
        if (useOOP)
            populateParticlesOOP(n);
        else if (useSoA)
            populateParticlesSoA(n);
        else
            populateParticlesAoS(n);
    };

    populateParticles(static_cast<sf::base::SizeT>(numEntities));

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
            // This is the bottleneck, consider reusing the particle instead of shifting/swapping
            if (destroyParticles)
            {
                const auto destroyPredicate = [](const float opacity) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                { return opacity <= 0.f; };

                if (useOOP)
                {
                    // TODO: erasure is the first issue!
                }
                else if (useSoA)
                {
                    if (destroyBySwapping)
                        particlesSoA.eraseIfBySwapping<5 /* opacity */>(destroyPredicate);
                    else
                        particlesSoA.eraseIfByShifting<5 /* opacity */>(destroyPredicate);
                }
                else
                {
                    if (destroyBySwapping)
                    {
                        sf::base::SizeT n = particlesAoS.size();
                        sf::base::SizeT i = 0;

                        while (i < n)
                        {
                            if (particlesAoS[i].opacity > 0.f)
                            {
                                ++i;
                                continue;
                            }

                            particlesAoS[i] = particlesAoS[--n];
                        }

                        particlesAoS.resize(n);
                    }
                    else
                    {
                        sf::base::vectorEraseIf(particlesAoS,
                                                [](const ParticleAoS& p) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                        { return p.opacity <= 0.f; });
                    }
                }
            }

            populateParticles(static_cast<sf::base::SizeT>(numEntities));

            if (!multithreadedUpdate)
            {
                if (useOOP)
                {
                    for (auto& e : entities)
                        e->update();
                }
                else if (useSoA)
                {
                    if (unifiedSoAProcessing)
                    {
                        particlesSoA.withAll(updateParticle);
                    }
                    else
                    {
                        particlesSoA.with<1, 2>([](sf::Vec2f& velocity, const sf::Vec2f& acc)
                                                    SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { velocity += acc; });

                        particlesSoA.with<0, 1>([](sf::Vec2f& position, sf::Vec2f& velocity)
                                                    SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { position += velocity; });

                        particlesSoA.with<3, 4>([](float& scale, const float scaleGrowth)
                                                    SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { scale += scaleGrowth; });

                        particlesSoA.with<5, 6>([](float& opacity, const float opacityGrowth)
                                                    SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { opacity += opacityGrowth; });

                        particlesSoA.with<7, 8>([](float& rotation, const float torque)
                                                    SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { rotation += torque; });
                    }
                }
                else
                {
                    for (ParticleAoS& p : particlesAoS)
                        updateParticle(p.position,
                                       p.velocity,
                                       p.acceleration,
                                       p.scale,
                                       p.scaleGrowth,
                                       p.opacity,
                                       p.opacityGrowth,
                                       p.rotation,
                                       p.torque);
                }
            }
            else
            {
                if (useOOP)
                {
                    // TODO
                }
                else if (useSoA)
                {
                    if (unifiedSoAProcessing)
                    {
                        doInBatches(static_cast<sf::base::SizeT>(numEntities),
                                    [&](const sf::base::SizeT /* iBatch */,
                                        const sf::base::SizeT batchStartIdx,
                                        const sf::base::SizeT batchEndIdx) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                        { particlesSoA.withAllSubRange(batchStartIdx, batchEndIdx, updateParticle); });
                    }
                    else
                    {
                        doInBatches(static_cast<sf::base::SizeT>(numEntities),
                                    [&](const sf::base::SizeT /* iBatch */,
                                        const sf::base::SizeT batchStartIdx,
                                        const sf::base::SizeT batchEndIdx) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                        {
                            particlesSoA.withSubRange<1, 2>(batchStartIdx,
                                                            batchEndIdx,
                                                            [](sf::Vec2f& velocity, const sf::Vec2f& acc)
                                                                SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                            { velocity += acc; });

                            particlesSoA.withSubRange<0, 1>(batchStartIdx,
                                                            batchEndIdx,
                                                            [](sf::Vec2f& position, sf::Vec2f& velocity)
                                                                SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                            { position += velocity; });

                            particlesSoA.withSubRange<3, 4>(batchStartIdx,
                                                            batchEndIdx,
                                                            [](float& scale, const float scaleGrowth)
                                                                SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                            { scale += scaleGrowth; });

                            particlesSoA.withSubRange<5, 6>(batchStartIdx,
                                                            batchEndIdx,
                                                            [](float& opacity, const float opacityGrowth)
                                                                SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                            { opacity += opacityGrowth; });

                            particlesSoA.withSubRange<7, 8>(batchStartIdx,
                                                            batchEndIdx,
                                                            [](float& rotation, const float torque)
                                                                SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                            { rotation += torque; });
                        });
                    }
                }
                else
                {
                    doInBatches(static_cast<sf::base::SizeT>(numEntities),
                                [&](const sf::base::SizeT /* iBatch */,
                                    const sf::base::SizeT batchStartIdx,
                                    const sf::base::SizeT batchEndIdx) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
                    {
                        for (sf::base::SizeT i = batchStartIdx; i < batchEndIdx; ++i)
                        {
                            ParticleAoS& p = particlesAoS[i];

                            updateParticle(p.position,
                                           p.velocity,
                                           p.acceleration,
                                           p.scale,
                                           p.scaleGrowth,
                                           p.opacity,
                                           p.opacityGrowth,
                                           p.rotation,
                                           p.torque);
                        }
                    });
                }
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

            constexpr const char* batchTypeItems[]{"Disabled",
                                                   "CPU Storage",
#ifndef SFML_OPENGL_ES
                                                   "GPU Storage"
#endif
            };

            if (ImGui::Combo("Batch type",
                             reinterpret_cast<int*>(&batchType),
                             batchTypeItems,
                             sf::base::getArraySize(batchTypeItems)))
                clearSamples();

            ImGui::BeginDisabled(batchType != BatchType::Disabled);
            if (ImGui::Combo("Autobatch type",
                             reinterpret_cast<int*>(&autobatchType),
                             batchTypeItems,
                             sf::base::getArraySize(batchTypeItems)))
            {
                clearSamples();
                window.setAutoBatchMode(static_cast<sf::RenderTarget::AutoBatchMode>(autobatchType));
            }

            {
                const sf::base::U64 step = 1u;
                ImGui::SetNextItemWidth(172.f);
                if (ImGui::InputScalar("Autobatch Vertex Threshold", ImGuiDataType_U64, &autoBatchVertexThreshold, &step))
                    window.setAutoBatchVertexThreshold(
                        static_cast<sf::base::SizeT>(sf::base::max(autoBatchVertexThreshold, sf::base::U64{1024u})));
            }
            ImGui::EndDisabled();

            ImGui::Checkbox("Multithreaded Update", &multithreadedUpdate);

            ImGui::BeginDisabled(batchType == BatchType::Disabled);
            ImGui::Checkbox("Multithreaded Draw", &multithreadedDraw);
            ImGui::EndDisabled();

            if (ImGui::Checkbox("Use OOP", &useOOP))
                populateParticles(static_cast<sf::base::SizeT>(numEntities));


            if (ImGui::Checkbox("Use SoA", &useSoA))
                populateParticles(static_cast<sf::base::SizeT>(numEntities));

            ImGui::Checkbox("Unified SoA processing", &unifiedSoAProcessing);
            ImGui::Checkbox("Destroy/recreate particles", &destroyParticles);
            ImGui::Checkbox("Destroy via swapping", &destroyBySwapping);
            ImGui::Checkbox("Draw step", &drawStep);

            {
                const sf::base::U64 step = 1u;
                ImGui::InputScalar("Workers", ImGuiDataType_U64, &nWorkers, &step);
            }
            nWorkers = sf::base::clamp(nWorkers, sf::base::U64{2u}, nMaxWorkers);

            ImGui::NewLine();

            ImGui::Text("Number of entities:");

            if (ImGui::InputInt("##InputInt", &numEntities))
                populateParticles(static_cast<sf::base::SizeT>(numEntities));

            ImGui::NewLine();

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

            ImGui::End();
        }
        // ---

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        if (drawStep)
        {
            window.clear();

            const sf::FloatRect& textureRect = spriteTextureRects[0];
            const auto           origin      = textureRect.size / 2.f;

            const auto drawNthParticle = [&](const sf::base::SizeT& i, auto&& drawFn)
            {
                if (useOOP)
                {
                    entities[i]->draw(textureAtlas.getTexture(), window);
                }
                else if (useSoA)
                {
                    particlesSoA.withNth<0, 3, 7>(i,
                                                  [&](const auto& position, const auto& scale, const auto& rotation)
                    {
                        drawParticleImpl(textureAtlas.getTexture(), textureRect, origin, position, scale, rotation, drawFn);
                    });
                }
                else
                {
                    const ParticleAoS& p = particlesAoS[i];
                    drawParticleImpl(textureAtlas.getTexture(), textureRect, origin, p.position, p.scale, p.rotation, drawFn);
                }
            };

            const auto doMultithreadedDraw = [&](auto& batchesArray)
            {
                for (auto& batch : batchesArray)
                    batch.clear();

                doInBatches(static_cast<sf::base::SizeT>(numEntities),
                            [&](const sf::base::SizeT iBatch, const sf::base::SizeT batchStartIdx, const sf::base::SizeT batchEndIdx)
                {
                    for (sf::base::SizeT i = batchStartIdx; i < batchEndIdx; ++i)
                        drawNthParticle(i,
                                        [&](const auto& drawable, const auto&...) { batchesArray[iBatch].add(drawable); });
                });

                for (auto& batch : batchesArray)
                    window.draw(batch, {.texture = &textureAtlas.getTexture()});
            };

            if (batchType == BatchType::Disabled || !multithreadedDraw)
            {
                cpuDrawableBatches[0].clear();
                gpuDrawableBatches[0].clear();

                for (sf::base::SizeT i = 0u; i < static_cast<sf::base::SizeT>(numEntities); ++i)
                    drawNthParticle(i,
                                    [&](const auto& drawable, const auto&... args)
                    {
                        if (batchType == BatchType::Disabled)
                            window.draw(drawable, args...);
                        else if (batchType == BatchType::CPUStorage)
                            cpuDrawableBatches[0].add(drawable);
                        else if (batchType == BatchType::GPUStorage)
                            gpuDrawableBatches[0].add(drawable);
                    });

                if (batchType == BatchType::CPUStorage)
                    window.draw(cpuDrawableBatches[0], {.texture = &textureAtlas.getTexture()});
                else if (batchType == BatchType::GPUStorage)
                    window.draw(gpuDrawableBatches[0], {.texture = &textureAtlas.getTexture()});
            }
            else if (batchType == BatchType::CPUStorage)
            {
                doMultithreadedDraw(cpuDrawableBatches);
            }
            else if (batchType == BatchType::GPUStorage)
            {
                // Must reserve in advance as reserving is not thread-safe
                for (sf::base::SizeT iBatch = 0u; iBatch < nMaxWorkers; ++iBatch)
                    gpuDrawableBatches[iBatch].reserveQuads(static_cast<sf::base::SizeT>(numEntities) / nWorkers * 2u);

                doMultithreadedDraw(gpuDrawableBatches);
            }
        }
        else
        {
            window.clear();
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
