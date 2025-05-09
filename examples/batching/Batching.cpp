#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
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
#include "SFML/Base/InterferenceSize.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ThreadPool.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <latch>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdio>
#include <cstdlib>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up random generator
    RNGFast rng(/* seed */ 1234);

    const auto getRndFloat = [&](const float min, const float max) { return rng.getF(min, max); };

    const auto getRndUInt = [&](const unsigned int min, const unsigned int max)
    { return rng.getI<unsigned int>(min, max); };

    const auto getRndU8 = [&](const sf::base::U8 min, const sf::base::U8 max)
    { return rng.getI<sf::base::U8>(min, max); };

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
        .title     = "Vittorio's SFML fork: batching example",
        .resizable = false,
        .vsync     = false,
    });

    //
    //
    // Set up imgui
    auto imGuiContext = sf::ImGui::ImGuiContext::create(window).value();

    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

    const auto addImgResourceToAtlas = [&](const sf::Path& path)
    { return textureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value(); };

    //
    //
    // Add white dot to atlas
    const auto whiteDotAtlasRect = textureAtlas.add(graphicsContext.getBuiltInWhiteDotTexture()).value();

    //
    //
    // Load fonts
    const auto fontTuffy        = sf::Font::openFromFile("resources/tuffy.ttf", &textureAtlas).value();
    const auto fontMouldyCheese = sf::Font::openFromFile("resources/mouldycheese.ttf", &textureAtlas).value();

    //
    //
    // Load images and add to texture atlas
    const sf::FloatRect spriteTextureRects[]{
        addImgResourceToAtlas("elephant.png"),
        addImgResourceToAtlas("giraffe.png"),
        addImgResourceToAtlas("monkey.png"),
        addImgResourceToAtlas("pig.png"),
        addImgResourceToAtlas("rabbit.png"),
        addImgResourceToAtlas("snake.png"),
    };

    //
    //
    // Simulation stuff
    struct Entity
    {
        sf::Text        text;
        sf::CircleShape circleShape;
        sf::Sprite      sprite;
        sf::Vec2f       velocity;
        float           torque;
    };

    std::vector<Entity> entities;

    const auto populateEntities = [&](const std::size_t n)
    {
        if (n < entities.size())
        {
            entities.erase(entities.begin() + static_cast<std::ptrdiff_t>(n), entities.end());
            return;
        }

        entities.clear();
        entities.reserve(n);

        char                  labelBuffer[64]{};
        constexpr const char* names[]{"Elephant", "Giraffe", "Monkey", "Pig", "Rabbit", "Snake"};

        for (std::size_t i = 0u; i < n; ++i)
        {
            const std::size_t    type        = i % 6u;
            const sf::FloatRect& textureRect = spriteTextureRects[type];

            std::snprintf(labelBuffer, 64, "%s #%zu", names[type], (i / (type + 1u)) + 1u);

            auto& [text, circleShape, sprite, velocity, torque] = entities.emplace_back(
                sf::Text{i % 2u == 0u ? fontTuffy : fontMouldyCheese,
                         {.string           = labelBuffer,
                          .fillColor        = sf::Color::Black,
                          .outlineColor     = sf::Color::White,
                          .outlineThickness = 5.f}},
                sf::CircleShape{
                    {.textureRect        = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
                     .outlineTextureRect = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
                     .fillColor    = {getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(125u, 255u)},
                     .outlineColor = {getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(125u, 255u)},
                     .outlineThickness = 3.f,
                     .radius           = getRndFloat(3.f, 8.f),
                     .pointCount       = getRndUInt(3u, 8u)}},
                sf::Sprite{.textureRect = textureRect},
                sf::Vec2f{getRndFloat(-2.5f, 2.5f), getRndFloat(-2.5f, 2.5f)},
                getRndFloat(-0.05f, 0.05f));

            sprite.origin   = textureRect.size / 2.f;
            sprite.rotation = sf::radians(getRndFloat(0.f, sf::base::tau));

            const float scaleFactor = getRndFloat(0.08f, 0.17f);
            sprite.scale            = {scaleFactor, scaleFactor};
            text.scale              = sprite.scale * 3.5f;

            sprite.position = {getRndFloat(0.f, windowSize.x), getRndFloat(0.f, windowSize.y)};

            text.origin        = text.getLocalBounds().getCenter();
            circleShape.origin = circleShape.getLocalBounds().getCenter();
        }
    };

    //
    //
    // Get hardware constants
    const auto     nMaxWorkers   = static_cast<sf::base::U64>(sf::base::ThreadPool::getHardwareWorkerCount());
    constexpr auto cacheLineSize = static_cast<std::size_t>(sf::base::hardwareDestructiveInterferenceSize);

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
    bool          drawSprites              = true;
    bool          drawText                 = true;
    bool          drawShapes               = true;
    bool          multithreadedUpdate      = false;
    bool          multithreadedDraw        = false;
    sf::base::U64 nWorkers                 = nMaxWorkers;
    int           numEntities              = 500;
    std::size_t   drawnVertices            = 0u;
    unsigned int  nDrawCalls               = 0u;

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

    std::vector<AlignedCPUDrawableBatch> cpuDrawableBatches(nMaxWorkers);
    std::vector<AlignedGPUDrawableBatch> gpuDrawableBatches(nMaxWorkers);

    //
    //
    // Set up thread pool
    sf::base::ThreadPool pool(nMaxWorkers);

    const auto doInBatches = [&](auto&& f)
    {
        const std::size_t entitiesPerBatch = entities.size() / nWorkers;

        std::latch latch{static_cast<std::ptrdiff_t>(nWorkers)};

        for (std::size_t i = 0u; i < nWorkers; ++i)
        {
            pool.post([&, i]
            {
                const std::size_t batchStartIdx = i * entitiesPerBatch;
                const std::size_t batchEndIdx   = (i == nWorkers - 1u) ? entities.size() : (i + 1u) * entitiesPerBatch;

                f(i, batchStartIdx, batchEndIdx);

                latch.count_down();
            });
        }

        latch.wait();
    };

    // Set up clock and time sampling
    sf::Clock clock;
    sf::Clock fpsClock;

    Sampler samplesEventMs(/* capacity */ 64u);
    Sampler samplesUpdateMs(/* capacity */ 64u);
    Sampler samplesImGuiMs(/* capacity */ 64u);
    Sampler samplesDrawMs(/* capacity */ 64u);
    Sampler samplesDisplayMs(/* capacity */ 64u);
    Sampler samplesFPS(/* capacity */ 64u);

    //
    //
    // Set up initial simulation state
    populateEntities(static_cast<std::size_t>(numEntities));

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
                    return EXIT_SUCCESS;
            }
        }
        samplesEventMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            const auto updateEntity = [&](Entity& entity)
            {
                auto& [text, circleShape, sprite, velocity, torque] = entity;

                sprite.position += velocity;
                sprite.rotation += sf::radians(torque);

                if ((sprite.position.x > windowSize.x && velocity.x > 0.f) || (sprite.position.x < 0.f && velocity.x < 0.f))
                    velocity.x = -velocity.x;

                if ((sprite.position.y > windowSize.y && velocity.y > 0.f) || (sprite.position.y < 0.f && velocity.y < 0.f))
                    velocity.y = -velocity.y;

                text.position = sprite.position - sf::Vec2f{0.f, 250.f * sprite.scale.x};

                circleShape.position = sprite.position;
                circleShape.rotation = sprite.rotation;
            };

            if (!multithreadedUpdate)
            {
                for (Entity& entity : entities)
                    updateEntity(entity);
            }
            else
            {
                doInBatches(
                    [&](const std::size_t /* iBatch */, const std::size_t batchStartIdx, const std::size_t batchEndIdx)
                {
                    for (std::size_t i = batchStartIdx; i < batchEndIdx; ++i)
                        updateEntity(entities[i]);
                });
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

            ImGui::Begin("Vittorio's SFML fork: batching example", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize({420.f, 490.f});

            const auto clearSamples = [&]
            {
                samplesEventMs.clear();
                samplesUpdateMs.clear();
                samplesImGuiMs.clear();
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

            const sf::base::U64 step = 1u;
            ImGui::SetNextItemWidth(172.f);
            if (ImGui::InputScalar("Autobatch Vertex Threshold", ImGuiDataType_U64, &autoBatchVertexThreshold, &step))
                window.setAutoBatchVertexThreshold(
                    static_cast<sf::base::SizeT>(sf::base::max(autoBatchVertexThreshold, sf::base::U64{1024u})));
            ImGui::EndDisabled();

            if (ImGui::Checkbox("Sprites", &drawSprites))
                clearSamples();

            ImGui::SameLine();
            if (ImGui::Checkbox("Texts", &drawText))
                clearSamples();

            ImGui::SameLine();
            if (ImGui::Checkbox("Shapes", &drawShapes))
                clearSamples();

            ImGui::Checkbox("Multithreaded Update", &multithreadedUpdate);

            ImGui::BeginDisabled(batchType == BatchType::Disabled);
            ImGui::Checkbox("Multithreaded Draw", &multithreadedDraw);
            ImGui::EndDisabled();

            ImGui::SetNextItemWidth(172.f);
            ImGui::InputScalar("Workers", ImGuiDataType_U64, &nWorkers, &step);
            nWorkers = sf::base::clamp(nWorkers, sf::base::U64{2u}, nMaxWorkers);

            ImGui::NewLine();

            ImGui::Text("Number of entities:");
            ImGui::SetNextItemWidth(172.f);
            ImGui::InputInt("##InputInt", &numEntities);

            if (ImGui::Button("Repopulate") && numEntities > 0)
            {
                populateEntities(static_cast<std::size_t>(numEntities));
                clearSamples();
            }

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
            // plotGraph("Events", " ms", samplesEventMs, 300.f);
            // plotGraph("ImGui", " ms", samplesImGuiMs, 300.f);
            plotGraph("Display", " ms", samplesDisplayMs, 300.f);

            ImGui::Spacing();
            ImGui::Text("Drawn vertices: %zu", drawnVertices);
            ImGui::Text("Draw calls: %u", nDrawCalls);

            ImGui::End();
        }
        samplesImGuiMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            window.clear();

            const auto drawEntity = [&](const Entity& entity, std::size_t& drawnVertexCounter, auto&& drawFn)
            {
                if (drawSprites)
                {
                    drawFn(entity.sprite, sf::RenderStates{.texture = &textureAtlas.getTexture()});
                    drawnVertexCounter += 4u;
                }

                if (drawText)
                {
                    drawFn(entity.text);
                    drawnVertexCounter += entity.text.getVertices().size();
                }

                if (drawShapes)
                {
                    drawFn(entity.circleShape, sf::RenderStates{.texture = &textureAtlas.getTexture()});

                    drawnVertexCounter += entity.circleShape.getFillVertices().size() +
                                          entity.circleShape.getOutlineVertices().size();
                }
            };

            const auto doMultithreadedDraw = [&](auto& batchesArray)
            {
                for (auto& batch : batchesArray)
                    batch.clear();

                // Initialize per-worker drawn vertex counts
                std::vector<std::size_t> totalChunkDrawnVertices(nMaxWorkers);

                doInBatches([&](const std::size_t iBatch, const std::size_t batchStartIdx, const std::size_t batchEndIdx)
                {
                    std::size_t chunkDrawnVertices = 0u; // avoid false sharing

                    for (std::size_t i = batchStartIdx; i < batchEndIdx; ++i)
                        drawEntity(entities[i],
                                   chunkDrawnVertices,
                                   [&](const auto& drawable, const auto&...) { batchesArray[iBatch].add(drawable); });

                    totalChunkDrawnVertices[iBatch] += chunkDrawnVertices;
                });

                drawnVertices = 0u;
                for (const auto v : totalChunkDrawnVertices)
                    drawnVertices += v;

                for (auto& batch : batchesArray)
                    window.draw(batch, {.texture = &textureAtlas.getTexture()});
            };

            if (batchType == BatchType::Disabled || !multithreadedDraw)
            {
                cpuDrawableBatches[0].clear();
                gpuDrawableBatches[0].clear();

                drawnVertices = 0u;

                for (const Entity& entity : entities)
                    drawEntity(entity,
                               drawnVertices,
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
                // Calculate reservation needed based on current state
                const sf::base::SizeT     maxEntitiesPerBatch       = (entities.size() + nWorkers - 1) / nWorkers;
                constexpr sf::base::SizeT maxQuadsPerEntityEstimate = 96u; // Safe upper bound
                const sf::base::SizeT     reservationSize           = maxEntitiesPerBatch * maxQuadsPerEntityEstimate;

                // Must reserve in advance as reserving is not thread-safe
                for (std::size_t iBatch = 0u; iBatch < nMaxWorkers; ++iBatch)
                    gpuDrawableBatches[iBatch].reserveQuads(reservationSize);

                doMultithreadedDraw(gpuDrawableBatches);
            }
        }
        samplesDrawMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        ////////////////////////////////////////////////////////////
        // Display step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
            imGuiContext.render(window);
            const auto stats = window.display();
            nDrawCalls       = stats.drawCalls;
        }
        samplesDisplayMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        samplesFPS.record(1.f / fpsClock.getElapsedTime().asSeconds());
    }
}
