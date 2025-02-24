#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
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
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ThreadPool.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <latch>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <cstddef>
#include <cstdio>


////////////////////////////////////////////////////////////
/// Sampler class
///
////////////////////////////////////////////////////////////
class Sampler
{
public:
    enum : unsigned int
    {
        ToIgnore   = 32u,
        MaxSamples = 512u
    };

    void record(float value)
    {
        if (m_toIgnore > 0u)
        {
            --m_toIgnore;
            return;
        }

        m_data.push_back(value);

        if (m_data.size() > MaxSamples)
            m_data.erase(m_data.begin());
    }

    [[nodiscard]] double getAverage() const
    {
        double accumulator = 0.0;

        for (auto value : m_data)
            accumulator += static_cast<double>(value);

        return accumulator / static_cast<double>(m_data.size());
    }

    [[nodiscard]] std::size_t size() const
    {
        return m_data.size();
    }

    [[nodiscard]] const float* data() const
    {
        return m_data.data();
    }

    void clear()
    {
        m_data.clear();
        m_toIgnore = ToIgnore;
    }

private:
    std::vector<float> m_data;
    unsigned int       m_toIgnore = ToIgnore;
};


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up random generator
    // std::minstd_rand rng(std::random_device{}());
    std::minstd_rand rng(100);
    const auto getRndFloat = [&](float min, float max) { return std::uniform_real_distribution<float>{min, max}(rng); };

    const auto getRndUInt = [&](unsigned int min, unsigned int max)
    { return std::uniform_int_distribution<unsigned int>{min, max}(rng); };

    const auto getRndU8 = [&](sf::base::U8 min, sf::base::U8 max)
    { return std::uniform_int_distribution<sf::base::U8>{min, max}(rng); };

    //
    //
    // Set up graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    constexpr sf::Vector2f windowSize{1680.f, 1050.f};

    sf::RenderWindow window(
        {.size = windowSize.toVector2u(), .title = "Vittorio's SFML fork: batching example", .resizable = false, .vsync = false});

    //
    //
    // Set up imgui
    sf::ImGui::ImGuiContext imGuiContext;
    if (!imGuiContext.init(window))
        return -1;

    //
    //
    // Set up texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

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
    // Load images
    const auto imgElephant = sf::Image::loadFromFile("resources/elephant.png").value();
    const auto imgGiraffe  = sf::Image::loadFromFile("resources/giraffe.png").value();
    const auto imgMonkey   = sf::Image::loadFromFile("resources/monkey.png").value();
    const auto imgPig      = sf::Image::loadFromFile("resources/pig.png").value();
    const auto imgRabbit   = sf::Image::loadFromFile("resources/rabbit.png").value();
    const auto imgSnake    = sf::Image::loadFromFile("resources/snake.png").value();

    //
    //
    // Add images to texture atlas
    const sf::FloatRect spriteTextureRects[]{textureAtlas.add(imgElephant).value(),
                                             textureAtlas.add(imgGiraffe).value(),
                                             textureAtlas.add(imgMonkey).value(),
                                             textureAtlas.add(imgPig).value(),
                                             textureAtlas.add(imgRabbit).value(),
                                             textureAtlas.add(imgSnake).value()};

    //
    //
    // Simulation stuff
    struct Entity
    {
        sf::Text        text;
        sf::Sprite      sprite;
        sf::Vector2f    velocity;
        float           torque;
        sf::CircleShape circleShape;
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

            std::snprintf(labelBuffer, 64, "%s #%zu", names[type], (i / (type + 1)) + 1);

            auto& [text, sprite, velocity, torque, circleShape] = entities.emplace_back(
                sf::Text{i % 2u == 0u ? fontTuffy : fontMouldyCheese,
                         {.string           = labelBuffer,
                          .fillColor        = sf::Color::Black,
                          .outlineColor     = sf::Color::White,
                          .outlineThickness = 5.f}},
                sf::Sprite{.textureRect = textureRect},
                sf::Vector2f{getRndFloat(-2.5f, 2.5f), getRndFloat(-2.5f, 2.5f)},
                getRndFloat(-0.05f, 0.05f),
                sf::CircleShape{
                    {.textureRect        = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
                     .outlineTextureRect = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
                     .fillColor    = {getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(125u, 255u)},
                     .outlineColor = {getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(0u, 255u), getRndU8(125u, 255u)},
                     .outlineThickness = 3.f,
                     .radius           = getRndFloat(3.f, 8.f),
                     .pointCount       = getRndUInt(3u, 8u)}});

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
    // Set up UI elements
    enum class BatchType : int
    {
        Disabled   = 0,
        CPUStorage = 1,
        GPUStorage = 2
    };

    auto        batchType           = BatchType::Disabled;
    bool        drawSprites         = true;
    bool        drawText            = true;
    bool        drawShapes          = true;
    bool        multithreadedUpdate = false;
    bool        multithreadedDraw   = false;
    int         numEntities         = 500;
    std::size_t drawnVertices       = 0u;

    //
    //
    // Set up drawable batches
    const sf::base::SizeT nWorkers = std::thread::hardware_concurrency();

    std::vector<sf::CPUDrawableBatch> cpuDrawableBatches(nWorkers);
    sf::PersistentGPUDrawableBatch    gpuDrawableBatch;

    //
    //
    // Set up thread pool
    sf::base::ThreadPool pool(nWorkers);

    // Set up clock and time sampling
    sf::Clock clock;
    sf::Clock fpsClock;

    Sampler samplesUpdateMs;
    Sampler samplesDrawMs;
    Sampler samplesFPS;

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
        while (sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        {
            clock.restart();

            const auto updateEntity = [&](Entity& entity)
            {
                auto& [text, sprite, velocity, torque, circleShape] = entity;

                sprite.position += velocity;
                sprite.rotation += sf::radians(torque);

                if ((sprite.position.x > windowSize.x && velocity.x > 0.f) || (sprite.position.x < 0.f && velocity.x < 0.f))
                    velocity.x = -velocity.x;

                if ((sprite.position.y > windowSize.y && velocity.y > 0.f) || (sprite.position.y < 0.f && velocity.y < 0.f))
                    velocity.y = -velocity.y;

                text.position = sprite.position - sf::Vector2f{0.f, 250.f * sprite.scale.x};

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
                const sf::base::SizeT entitiesPerBatch = entities.size() / nWorkers;

                std::latch latch{static_cast<std::ptrdiff_t>(nWorkers)};

                for (std::size_t i = 0u; i < nWorkers; ++i)
                {
                    pool.post([&, i]
                    {
                        const sf::base::SizeT batchStartIdx = i * entitiesPerBatch;
                        const sf::base::SizeT batchEndIdx = (i == nWorkers - 1u) ? entities.size() : (i + 1u) * entitiesPerBatch;

                        for (sf::base::SizeT j = batchStartIdx; j < batchEndIdx; ++j)
                            updateEntity(entities[j]);

                        latch.count_down();
                    });
                }

                latch.wait();
            }

            samplesUpdateMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        }

        ////////////////////////////////////////////////////////////
        // ImGui step
        ////////////////////////////////////////////////////////////
        {
            imGuiContext.update(window, fpsClock.getElapsedTime());

            ImGui::Begin("Vittorio's SFML fork: batching example", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize(ImVec2{350.f, 304.f});

            const auto clearSamples = [&]
            {
                samplesUpdateMs.clear();
                samplesDrawMs.clear();
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

            if (ImGui::Checkbox("Sprites", &drawSprites))
                clearSamples();

            ImGui::SameLine();
            if (ImGui::Checkbox("Texts", &drawText))
                clearSamples();

            ImGui::SameLine();
            if (ImGui::Checkbox("Shapes", &drawShapes))
                clearSamples();

            ImGui::NewLine();

            ImGui::Checkbox("Multithreaded Update", &multithreadedUpdate);
            ImGui::SameLine();

            ImGui::BeginDisabled(batchType != BatchType::CPUStorage);
            ImGui::Checkbox("Multithreaded Draw", &multithreadedDraw);
            ImGui::EndDisabled();

            ImGui::NewLine();

            ImGui::Text("Number of entities:");
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

            plotGraph("Update (ms)", " ms", samplesUpdateMs, 10.f);
            plotGraph("Draw (ms)", " ms", samplesDrawMs, 100.f);
            plotGraph("FPS", " FPS", samplesFPS, 300.f);

            ImGui::Spacing();
            ImGui::Text("Drawn vertices: %zu", drawnVertices);

            ImGui::End();
        }

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        {
            clock.restart();

            window.clear();

            const auto drawEntity = [&](const Entity& entity, std::size_t& drawnVertexCounter, auto&& drawFn)
            {
                if (drawSprites)
                {
                    drawFn(entity.sprite, textureAtlas.getTexture());
                    drawnVertexCounter += 4u;
                }

                if (drawText)
                {
                    drawFn(entity.text);
                    drawnVertexCounter += entity.text.getVertices().size();
                }

                if (drawShapes)
                {
                    drawFn(entity.circleShape, &textureAtlas.getTexture());

                    drawnVertexCounter += entity.circleShape.getFillVertices().size() +
                                          entity.circleShape.getOutlineVertices().size();
                }
            };

            if (batchType != BatchType::CPUStorage || !multithreadedDraw)
            {
                cpuDrawableBatches[0].clear();
                gpuDrawableBatch.clear();

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
                            gpuDrawableBatch.add(drawable);
                    });

                if (batchType == BatchType::CPUStorage)
                    window.draw(cpuDrawableBatches[0], {.texture = &textureAtlas.getTexture()});
                else if (batchType == BatchType::GPUStorage)
                    window.draw(gpuDrawableBatch, {.texture = &textureAtlas.getTexture()});

                samplesDrawMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
            }
            else
            {
                for (auto& batch : cpuDrawableBatches)
                    batch.clear();

                // Initialize per-worker drawn vertex counts
                std::vector<std::size_t> totalChunkDrawnVertices(nWorkers);

                const sf::base::SizeT entitiesPerBatch = entities.size() / nWorkers;

                std::latch latch{static_cast<std::ptrdiff_t>(nWorkers)};

                for (std::size_t i = 0u; i < nWorkers; ++i)
                {
                    pool.post([&, i]
                    {
                        std::size_t chunkDrawnVertices = 0u;

                        const sf::base::SizeT batchStartIdx = i * entitiesPerBatch;
                        const sf::base::SizeT batchEndIdx = (i == nWorkers - 1u) ? entities.size() : (i + 1u) * entitiesPerBatch;

                        for (sf::base::SizeT j = batchStartIdx; j < batchEndIdx; ++j)
                            drawEntity(entities[j],
                                       chunkDrawnVertices,
                                       [&](const auto& drawable, const auto&...) { cpuDrawableBatches[i].add(drawable); });

                        totalChunkDrawnVertices[i] += chunkDrawnVertices;
                        latch.count_down();
                    });
                }

                latch.wait();

                drawnVertices = 0u;
                for (const auto v : totalChunkDrawnVertices)
                    drawnVertices += v;

                for (auto& batch : cpuDrawableBatches)
                    window.draw(batch, {.texture = &textureAtlas.getTexture()});

                samplesDrawMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
            }

            imGuiContext.render(window);
            window.display();

            samplesFPS.record(1.f / fpsClock.getElapsedTime().asSeconds());

            window.setTitle("FPS: " + std::to_string(samplesFPS.getAverage()) +
                            " || U: " + std::to_string(samplesUpdateMs.getAverage()) +
                            " || D: " + std::to_string(samplesDrawMs.getAverage()));
        }
    }
}
