#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGui.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <initializer_list>
#include <string>

#include <cmath>
#include <cstdio>


////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1016.f, 1016.f};

////////////////////////////////////////////////////////////
class ExampleShapes
{
private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow& m_window;
    const sf::Font&   m_font;

    ////////////////////////////////////////////////////////////
    float m_time  = 0.f;
    float m_phase = 0.f;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] auto applyCommonSettings(const sf::Vec2f currentOffset, auto shapeData)
    {
        const auto fillColor = sf::Color::Red.withRotatedHue(m_time + m_phase * 65.f);

        shapeData.position += currentOffset;
        shapeData.position += {64.f, 64.f};

        shapeData.fillColor        = fillColor;
        shapeData.outlineColor     = fillColor.withRotatedHue(180.f);
        shapeData.outlineThickness = std::abs(4.f * std::sin(m_time * 0.05f + m_phase));

        shapeData.rotation = sf::degrees(std::fmod(m_time * 1.f + m_phase * 45.f, 360.f));

        shapeData.textureRect = {
            .position = {0.f, 0.f},
            .size     = {1.f, 1.f},
        };

        return shapeData;
    }

    ////////////////////////////////////////////////////////////
    void drawShape(const sf::Vec2f currentOffset, const char* label, const auto& shapeData)
    {
        m_window.draw(applyCommonSettings(currentOffset, shapeData), {.texture = &m_font.getTexture()});

        m_window.draw(m_font,
                      sf::TextData{
                          .position         = shapeData.position + currentOffset,
                          .string           = label,
                          .characterSize    = 16,
                          .outlineColor     = sf::Color::Black,
                          .outlineThickness = 2.f,
                      });

        m_phase += 0.1f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getPhasedValue(const float timeMultiplier, const float phaseMultiplier) const
    {
        return std::abs(std::sin(m_time * timeMultiplier + m_phase * phaseMultiplier));
    }

    ////////////////////////////////////////////////////////////
    void drawAllShapes(const sf::Vec2f offset)
    {
        drawShape(offset,
                  "Circle",
                  sf::CircleShapeData{
                      .position   = {32.f, 32.f},
                      .origin     = {64.f, 64.f},
                      .radius     = 64.f,
                      .pointCount = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.04f, 2.f)),
                  });

        drawShape(offset,
                  "Ellipse",
                  sf::EllipseShapeData{
                      .position         = {196.f, 32.f},
                      .origin           = {64.f, 32.f},
                      .horizontalRadius = 64.f,
                      .verticalRadius   = 32.f,
                      .pointCount       = 3u + static_cast<unsigned int>(29.f * getPhasedValue(0.06f, 3.5f)),
                  });

        drawShape(offset,
                  "PieSlice",
                  sf::PieSliceShapeData{
                      .position   = {364.f, 32.f},
                      .origin     = {64.f, 64.f},
                      .radius     = 64.f,
                      .startAngle = sf::degrees(0.f),
                      .sweepAngle = sf::degrees((360.f * getPhasedValue(0.1f, 2.f))),
                      .pointCount = 32u,
                  });

        drawShape(offset,
                  "Rectangle",
                  sf::RectangleShapeData{
                      .position = {32.f, 196.f},
                      .origin   = {64.f, 32.f},
                      .size     = {128.f, 64.f},
                  });

        drawShape(offset,
                  "RoundedRectangle",
                  sf::RoundedRectangleShapeData{
                      .position         = {196.f, 196.f},
                      .origin           = {64.f, 32.f},
                      .size             = {128.f, 64.f},
                      .cornerRadius     = 3.f + (29.f * getPhasedValue(0.1f, 1.5f)),
                      .cornerPointCount = 16u,
                  });

        drawShape(offset,
                  "RingPieSlice",
                  sf::RingPieSliceShapeData{
                      .position    = {364.f, 196.f},
                      .origin      = {64.f, 64.f},
                      .outerRadius = 64.f,
                      .innerRadius = 32.f + (16.f * getPhasedValue(0.2f, 0.75f)),
                      .startAngle  = sf::degrees(0.f),
                      .sweepAngle  = sf::degrees((360.f * getPhasedValue(0.1f, 2.0f))),
                      .pointCount  = 32u,
                  });

        drawShape(offset,
                  "Ring",
                  sf::RingShapeData{
                      .position    = {364.f, 364.f},
                      .origin      = {64.f, 64.f},
                      .outerRadius = 64.f,
                      .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                      .pointCount  = 30u,
                  });

        drawShape(offset,
                  "Star",
                  sf::StarShapeData{
                      .position    = {32.f, 364.f},
                      .origin      = {64.f, 64.f},
                      .outerRadius = 64.f,
                      .innerRadius = 32.f + (16.f * getPhasedValue(0.25f, 2.f)),
                      .pointCount  = 3u + static_cast<unsigned int>(10.f * getPhasedValue(0.1f, 2.f)),
                  });

        drawShape(offset,
                  "Arrow",
                  sf::ArrowShapeData{
                      .position    = {196.f, 364.f},
                      .origin      = {(64.f + 48.f) / 2.f, 0.f},
                      .shaftLength = 64.f,
                      .shaftWidth  = 32.f + (32.f * getPhasedValue(0.04f, 2.f)),
                      .headLength  = 48.f,
                      .headWidth   = 96.f - (64.f * getPhasedValue(0.06f, 3.f)),
                  });
    }

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleShapes(sf::RenderWindow& window, const sf::Font& font) : m_window{window}, m_font{font}
    {
    }

    ////////////////////////////////////////////////////////////
    void update(const float deltaTimeMs)
    {
        m_time += deltaTimeMs;
    }

    ////////////////////////////////////////////////////////////
    void draw()
    {
        m_phase = 0.f;

        for (const auto offset : {sf::Vec2f{0.f, 0.f}, {492.f, 0.f}, {0.f, 492.f}, {492.f, 492.f}})
            drawAllShapes(offset);
    }
};


////////////////////////////////////////////////////////////
class ExampleBunnyMark
{
private:
    ////////////////////////////////////////////////////////////
    struct Bunny
    {
        sf::Vec2f position;
        sf::Vec2f velocity;
        sf::Angle rotation;
        float     scale{};
    };

    ////////////////////////////////////////////////////////////
    sf::RenderWindow&       m_window;
    const sf::Font&         m_font;
    const sf::TextureAtlas& m_textureAtlas;
    const sf::FloatRect (&m_bunnyTextureRects)[8];

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<Bunny> m_bunnies;
    sf::base::SizeT         m_bunnyTargetCount = 100'000u;

    ////////////////////////////////////////////////////////////
    RNGFast m_rng{/* seed */ 1234};

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::string toDigitSeparatedString(const sf::base::SizeT value)
    {
        std::string s = std::to_string(value);

        for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
            s.insert(static_cast<sf::base::SizeT>(i), ".");

        return s;
    }

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleBunnyMark(sf::RenderWindow&       window,
                              const sf::Font&         font,
                              const sf::TextureAtlas& textureAtlas,
                              const sf::FloatRect (&bunnyTextureRects)[8]) :
    m_window{window},
    m_font{font},
    m_textureAtlas{textureAtlas},
    m_bunnyTextureRects{bunnyTextureRects}
    {
    }

    ////////////////////////////////////////////////////////////
    void update(const float deltaTimeMs)
    {
        m_time += deltaTimeMs;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            m_bunnyTargetCount += 1000;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            m_bunnyTargetCount -= 1000;

        m_bunnyTargetCount = sf::base::clamp(m_bunnyTargetCount, sf::base::SizeT{1000}, sf::base::SizeT{2'500'000});

        if (m_bunnies.size() < m_bunnyTargetCount)
        {
            m_bunnies.reserve(m_bunnyTargetCount);

            for (sf::base::SizeT i = m_bunnies.size(); i < m_bunnyTargetCount; ++i)
            {
                m_bunnies.emplaceBack(
                    /* position */ m_rng.getVec2f(resolution),
                    /* velocity */ m_rng.getVec2f({-1.f, -1.f}, {1.f, 1.f}),
                    /* rotation */ sf::radians(m_rng.getF(0.f, sf::base::tau)),
                    /*    scale */ m_rng.getF(0.25f, 0.5f));
            }
        }
        else if (m_bunnies.size() > m_bunnyTargetCount)
        {
            m_bunnies.resize(m_bunnyTargetCount);
        }

        for (auto& [position, velocity, rotation, scale] : m_bunnies)
        {
            position += velocity * deltaTimeMs;

            if (position.x < 0.f)
                position.x = resolution.x;
            else if (position.x > resolution.x)
                position.x = 0.f;

            if (position.y < 0.f)
                position.y = resolution.y;
            else if (position.y > resolution.y)
                position.y = 0.f;

            rotation += sf::radians(0.05f * deltaTimeMs);
        }
    }

    ////////////////////////////////////////////////////////////
    void draw()
    {
        sf::base::SizeT i = 0;

        for (auto& [position, velocity, rotation, scale] : m_bunnies)
        {
            const auto& txr = m_bunnyTextureRects[i % 8u];

            m_window.draw(
                sf::Sprite{
                    .position    = position,
                    .scale       = {scale, scale},
                    .origin      = txr.size / 2.f,
                    .rotation    = rotation,
                    .textureRect = txr,
                },
                {.texture = &m_textureAtlas.getTexture()});

            ++i;
        }

        const auto digitSeparatedBunnyCount = toDigitSeparatedString(m_bunnies.size());

        const auto vertices = m_window.draw(m_font,
                                            sf::TextData{
                                                .position         = {8.f, 8.f},
                                                .string           = digitSeparatedBunnyCount + " bunnies",
                                                .characterSize    = 32,
                                                .outlineColor     = sf::Color::Black,
                                                .outlineThickness = 4.f,
                                            });

        for (sf::base::SizeT j = 0u; j < vertices.size(); j += 4u)
        {
            const sf::base::SizeT outlineIndependentIndex = j % (vertices.size() / 2u);

            if (outlineIndependentIndex >= digitSeparatedBunnyCount.size() * 4u)
            {
                const float offY = std::sin(m_time) * 1.25f;

                vertices[j + 0].position.y -= offY;
                vertices[j + 1].position.y -= offY;
                vertices[j + 2].position.y += offY;
                vertices[j + 3].position.y += offY;

                vertices[j + 0].position.x -= offY;
                vertices[j + 1].position.x += offY;
                vertices[j + 2].position.x -= offY;
                vertices[j + 3].position.x += offY;
            }
            else
            {
                const float offY = std::sin(m_time + static_cast<float>(outlineIndependentIndex)) * 1.5f;

                vertices[j + 0].position.y += offY;
                vertices[j + 1].position.y += offY;
                vertices[j + 2].position.y += offY;
                vertices[j + 3].position.y += offY;
            }
        }

        m_window.draw(m_font,
                      sf::TextData{
                          .position         = {8.f, 48.f},
                          .string           = "Change number of bunnies with arrow keys",
                          .characterSize    = 16,
                          .outlineColor     = sf::Color::Black,
                          .outlineThickness = 2.f,
                      });
    }
};


////////////////////////////////////////////////////////////
class ExampleAudio
{
    // TODO P0: implement

private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow& m_window;
    const sf::Font&   m_font;

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    sf::SoundBuffer m_sbByteMeow  = sf::SoundBuffer::loadFromFile("resources/bytemeow.ogg").value();
    sf::MusicReader m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwizard.mp3").value();

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::PlaybackDevice, 8> m_playbackDevices;

    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Sound, 32> m_activeSounds;
    sf::base::Optional<sf::Music>          m_activeMusic;

    ////////////////////////////////////////////////////////////
    void refreshPlaybackDevices()
    {
        // Sounds and musics must be destroyed *before* playback devices
        m_activeSounds.clear();
        m_activeMusic.reset();

        m_playbackDevices.clear();

        for (const auto& playbackDeviceHandle : sf::AudioContext::getAvailablePlaybackDeviceHandles())
            m_playbackDevices.emplaceBack(playbackDeviceHandle);
    }

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleAudio(sf::RenderWindow& window, const sf::Font& font) : m_window{window}, m_font{font}
    {
        refreshPlaybackDevices();
    }

    ////////////////////////////////////////////////////////////
    void update(const float deltaTimeMs)
    {
        m_time += deltaTimeMs;
    }

    ////////////////////////////////////////////////////////////
    void imgui()
    {
        ImGui::Begin("Audio Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Playback devices:");

        sf::base::SizeT i = 0u;
        for (auto& playbackDevice : m_playbackDevices)
        {
            ImGui::Text("%s", playbackDevice.getName());
            ImGui::SameLine();
            ImGui::Text("%s", playbackDevice.isDefault() ? "(default)" : "");
            ImGui::SameLine();

            char buttonLabel[64];

            std::snprintf(buttonLabel, sizeof(buttonLabel), "Play Sound##%zu", i);
            if (ImGui::Button(buttonLabel))
            {
                auto* const it = sf::base::findIf( //
                    m_activeSounds.begin(),
                    m_activeSounds.end(),
                    [](const sf::Sound& sound) { return !sound.isPlaying(); });

                if (it != m_activeSounds.end())
                {
                    if (&it->getPlaybackDevice() == &playbackDevice)
                        it->play();
                    else
                        m_activeSounds.reEmplaceByIterator(it, playbackDevice, m_sbByteMeow).play();
                }

                if (m_activeSounds.size() < 32u)
                    m_activeSounds.emplaceBack(playbackDevice, m_sbByteMeow).play();
            }

            ImGui::SameLine();

            std::snprintf(buttonLabel, sizeof(buttonLabel), "Play Music##%zu", i);
            if (ImGui::Button(buttonLabel))
            {
                if (!m_activeMusic.hasValue())
                    m_activeMusic.emplace(playbackDevice, m_msBGMWizard).play();
                else if (&m_activeMusic->getPlaybackDevice() == &playbackDevice)
                    m_activeMusic->resume();
                else
                {
                    const auto playingOffset = m_activeMusic->getPlayingOffset();
                    m_activeMusic.emplace(playbackDevice, m_msBGMWizard).play();
                    m_activeMusic->setPlayingOffset(playingOffset);
                }
            }

            ++i;
        }

        if (ImGui::Button("Refresh playback devices"))
            refreshPlaybackDevices();

        ImGui::SameLine();

        if (ImGui::Button("Pause Music"))
        {
            if (m_activeMusic.hasValue())
                m_activeMusic->pause();
        }

        ImGui::SameLine();

        if (ImGui::Button("Stop Music"))
        {
            m_activeMusic.reset();
        }

        static bool x = false;
        if (ImGui::Button("Switch Music Source"))
        {
            if (x)
                m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwizard.mp3").value();
            else
                m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwitch.mp3").value();

            x = !x;
        }

        ImGui::End();
    }

    ////////////////////////////////////////////////////////////
    void draw()
    {
    }
};


////////////////////////////////////////////////////////////
class Game
{
private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow&        m_window;
    sf::ImGui::ImGuiContext& m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::Clock m_clock;
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    Sampler m_samplesEventMs{/* capacity */ 64u};
    Sampler m_samplesUpdateMs{/* capacity */ 64u};
    Sampler m_samplesImGuiMs{/* capacity */ 64u};
    Sampler m_samplesDrawMs{/* capacity */ 64u};
    Sampler m_samplesDisplayMs{/* capacity */ 64u};
    Sampler m_samplesFPS{/* capacity */ 64u};

    ////////////////////////////////////////////////////////////
    unsigned int    m_lastFrameDrawCallCount = 0u;
    sf::base::SizeT m_lastFrameDrawnVertices = 0u;

    ////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({2048u, 2048u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    const sf::FloatRect m_txrWhiteDot = m_textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value();

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/tuffy.ttf", &m_textureAtlas).value();

    ////////////////////////////////////////////////////////////
    const sf::FloatRect m_bunnyTextureRects[8] = {
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 0.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 45.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 90.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 135.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 180.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 225.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 270.f),
        addImgToAtlasWithRotatedHue("resources/bunny0.png", 315.f),
    };

    ////////////////////////////////////////////////////////////
    ExampleShapes    m_exampleShapes{m_window, m_font};
    ExampleBunnyMark m_exampleBunnyMark{m_window, m_font, m_textureAtlas, m_bunnyTextureRects};
    ExampleAudio     m_exampleAudio{m_window, m_font};

    ////////////////////////////////////////////////////////////
    sf::base::SizeT m_activeExample = 2u;

    ////////////////////////////////////////////////////////////
    void clearSamples()
    {
        m_samplesEventMs.clear();
        m_samplesUpdateMs.clear();
        m_samplesImGuiMs.clear();
        m_samplesDrawMs.clear();
        m_samplesDisplayMs.clear();
        m_samplesFPS.clear();
    }

    ////////////////////////////////////////////////////////////
    void plotSamples(const char* label, const char* unit, const Sampler& samples, float upperBound)
    {
        ImGui::PlotLines(label,
                         samples.data(),
                         static_cast<int>(samples.size()),
                         0,
                         (std::to_string(samples.getAverage()) + unit).c_str(),
                         0.f,
                         upperBound,
                         ImVec2{256.f, 32.f});
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile(path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgToAtlasWithRotatedHue(const sf::Path& path, const float hueDegrees)
    {
        auto img = sf::Image::loadFromFile(path).value();
        img.rotateHue(hueDegrees);
        return m_textureAtlas.add(img).value();
    }

public:
    ////////////////////////////////////////////////////////////
    explicit Game(sf::RenderWindow& window, sf::ImGui::ImGuiContext& imGuiContext) :
    m_window{window},
    m_imGuiContext{imGuiContext}
    {
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            ////////////////////////////////////////////////////////////
            // Event handling
            ////////////////////////////////////////////////////////////
            // ---
            m_clock.restart();

            while (sf::base::Optional event = m_window.pollEvent())
            {
                m_imGuiContext.processEvent(m_window, *event);

                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return true;

                if (handleAspectRatioAwareResize(*event, resolution, m_window))
                    continue;

                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                    if (eKeyPressed->code == sf::Keyboard::Key::Space)
                        m_activeExample = ((m_activeExample + 1u) % 3u);
            }

            m_samplesEventMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Update step
            ////////////////////////////////////////////////////////////
            // ---
            m_clock.restart();

            const auto  deltaTime   = m_fpsClock.restart();
            const float deltaTimeMs = deltaTime.asSeconds() * 1000.f;

            if (m_activeExample == 0u)
                m_exampleShapes.update(deltaTimeMs * 0.01f);
            else if (m_activeExample == 1u)
                m_exampleBunnyMark.update(deltaTimeMs * 0.01f);
            else if (m_activeExample == 2u)
                m_exampleAudio.update(deltaTimeMs * 0.01f);

            m_samplesUpdateMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // ImGui step
            ////////////////////////////////////////////////////////////
            // ---
            m_clock.restart();

            m_imGuiContext.update(m_window, deltaTime);

            ImGui::Begin("Granita Showcase", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            // ImGui::SetWindowSize({420.f, 490.f});

            plotSamples("Update", " ms", m_samplesUpdateMs, 64.f);
            plotSamples("Draw", " ms", m_samplesDrawMs, 64.f);
            plotSamples("FPS", " FPS", m_samplesFPS, 360.f);
            // plotSamples("Events", " ms", m_samplesEventMs, 64.f);
            // plotSamples("ImGui", " ms", m_samplesImGuiMs, 64.f);
            plotSamples("Display", " ms", m_samplesDisplayMs, 64.f);

            ImGui::Spacing();
            ImGui::Text("Draw calls: %u", m_lastFrameDrawCallCount);
            ImGui::Text("Drawn vertices: %zu", m_lastFrameDrawnVertices);

            ImGui::End();

            if (m_activeExample == 2u)
                m_exampleAudio.imgui();

            m_samplesImGuiMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Draw step
            ////////////////////////////////////////////////////////////
            // ---
            m_clock.restart();

            m_window.clear();

            if (m_activeExample == 0u)
                m_exampleShapes.draw();
            else if (m_activeExample == 1u)
                m_exampleBunnyMark.draw();
            else if (m_activeExample == 2u)
                m_exampleAudio.draw();

            m_samplesDrawMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Display step
            ////////////////////////////////////////////////////////////
            // ---
            m_clock.restart();
            {
                m_imGuiContext.render(m_window);

                const auto [drawCalls, drawnVertices] = m_window.display();

                m_lastFrameDrawCallCount = drawCalls;
                m_lastFrameDrawnVertices = drawnVertices;
            }
            m_samplesDisplayMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
            // ---
            ////////////////////////////////////////////////////////////

            m_samplesFPS.record(1.f / m_fpsClock.getElapsedTime().asSeconds());
        }

        return true;
    }
};

////////////////////////////////////////////////////////////
int main()
{
    //
    //
    // Set up audio and graphics contexts
    auto audioContext    = sf::AudioContext::create().value();
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    auto window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVec2u(),
         .title           = "Showcase",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 8u}});

    //
    //
    // Set up imgui
    auto imGuiContext = sf::ImGui::ImGuiContext::create(window).value(); // TODO P0: use same pattern as other contexts

    //
    //
    // Set up game and simulation loop
    Game game{window, imGuiContext};

    if (!game.run())
        return 1;

    return 0;
}
