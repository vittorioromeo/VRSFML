#include "ShowcaseAudio.hpp"
#include "ShowcaseBunnyMark.hpp"
#include "ShowcaseExample.hpp"
#include "ShowcaseIndividualShape.hpp"
#include "ShowcaseShapes.hpp"
#include "ShowcaseTextComparison.hpp"

#include "ExampleUtils/Sampler.hpp"
#include "ExampleUtils/Scaling.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"
#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Audio/AudioContext.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"


namespace
{
////////////////////////////////////////////////////////////
class Game
{
private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow m_window = makeDPIScaledRenderWindow(
                                    {
                                        .size           = resolution.toVec2u(),
                                        .title          = "Showcase",
                                        .resizable      = true,
                                        .vsync          = true,
                                        .frametimeLimit = 144u,
                                    })
                                    .value();

    ////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtGame = makeAARenderTexture(resolution.toVec2u(), {.antiAliasingLevel = 8u, .smooth = true}).value();

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::View m_worldView  = sf::View::fromScreenSize(resolution);
    sf::View m_windowView = sf::View::fromScreenSize(resolution);

    ////////////////////////////////////////////////////////////
    sf::Clock m_clock;
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    Sampler<float> m_samplesEventMs{/* capacity */ 64u};
    Sampler<float> m_samplesUpdateMs{/* capacity */ 64u};
    Sampler<float> m_samplesImGuiMs{/* capacity */ 64u};
    Sampler<float> m_samplesDrawMs{/* capacity */ 64u};
    Sampler<float> m_samplesDisplayMs{/* capacity */ 64u};
    Sampler<float> m_samplesFPS{/* capacity */ 64u};

    ////////////////////////////////////////////////////////////
    unsigned int    m_lastFrameDrawCallCount = 0u;
    sf::base::SizeT m_lastFrameDrawnVertices = 0u;

    ////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    ImFont*        m_imGuiFont{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/Born2bSportyFS.ttf", 18.f)};
    const sf::Font m_font = sf::Font::openFromFile("resources/tuffy.ttf", &m_textureAtlas).value();

    ////////////////////////////////////////////////////////////
    GameDependencies m_deps{&m_window, &m_rtGame, &m_worldView, &m_font};

    ////////////////////////////////////////////////////////////
    ExampleShapes          m_exampleShapes{m_deps};
    ExampleBunnyMark       m_exampleBunnyMark{m_deps, m_textureAtlas};
    ExampleAudio           m_exampleAudio;
    ExampleIndividualShape m_exampleIndividualShape{m_deps};
    ExampleTextComparison  m_exampleTextComparison{m_deps};

    ////////////////////////////////////////////////////////////
    static constexpr int exampleCount = 5;

    ////////////////////////////////////////////////////////////
    ShowcaseExample* m_examples[exampleCount]{
        &m_exampleShapes,
        &m_exampleBunnyMark,
        &m_exampleAudio,
        &m_exampleIndividualShape,
        &m_exampleTextComparison,
    };

    ////////////////////////////////////////////////////////////
    int m_activeExample = 3;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] ShowcaseExample& getActiveExample()
    {
        return *m_examples[m_activeExample];
    }


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
    void plotSamples(const char* label, const char* unit, const Sampler<float>& samples, float upperBound)
    {
        ImGui::PlotLines(label,
                         samples.data(),
                         static_cast<int>(samples.size()),
                         0,
                         (sf::base::toString(samples.getAverageAs<double>()) + unit).cStr(),
                         0.f,
                         upperBound,
                         ImVec2{256.f, 32.f});
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f addImgToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile(path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool runEventHandling()
    {
        m_clock.restart();

        while (sf::base::Optional event = m_window.pollEvent())
        {
            m_imGuiContext.processEvent(m_window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return true;

            if (handleAspectRatioAwareResize(*event, resolution, m_windowView))
                continue;

            if (ImGui::GetIO().WantCaptureKeyboard)
                continue;

            if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                if (eKeyPressed->code == sf::Keyboard::Key::Space)
                {
                    ++m_activeExample;

                    if (m_activeExample >= exampleCount)
                        m_activeExample = 0;
                }
        }

        m_samplesEventMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);

        return false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Time runUpdate()
    {
        m_clock.restart();

        const sf::Time deltaTime   = m_fpsClock.restart();
        const float    deltaTimeMs = deltaTime.asSeconds() * 1000.f;

        getActiveExample().update(deltaTimeMs * 0.01f);

        m_samplesUpdateMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);

        return deltaTime;
    }

    ////////////////////////////////////////////////////////////
    void runImGui(sf::Time deltaTime)
    {
        m_clock.restart();

        m_imGuiContext.update(m_window, deltaTime);

        ImGui::PushFont(m_imGuiFont);
        ImGui::Begin("Granita Showcase", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::SetNextItemWidth(120.f);
        {
            const char* names[exampleCount];
            for (sf::base::SizeT i = 0u; i < exampleCount; ++i)
                names[i] = m_examples[i]->name;

            ImGui::Combo("Example", &m_activeExample, names, exampleCount);
        }

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

        getActiveExample().imgui();

        ImGui::PopFont();

        m_samplesImGuiMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
    }

    ////////////////////////////////////////////////////////////
    void runDraw()
    {
        m_clock.restart();

        m_rtGame.clear();
        getActiveExample().draw();

        m_samplesDrawMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
    }

    ////////////////////////////////////////////////////////////
    void runDisplay()
    {
        m_clock.restart();

        const auto [drawCalls, drawnVertices] = m_rtGame.display();

        m_lastFrameDrawCallCount = drawCalls;
        m_lastFrameDrawnVertices = drawnVertices;

        m_window.clear();
        m_window.draw(m_rtGame.getTexture(), {.view = m_windowView});
        m_imGuiContext.render(m_window);
        m_window.display();

        m_samplesDisplayMs.record(m_clock.getElapsedTime().asSeconds() * 1000.f);
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            if (runEventHandling())
                return true;

            const sf::Time deltaTime = runUpdate();

            runImGui(deltaTime);
            runDraw();
            runDisplay();

            m_samplesFPS.record(1.f / m_fpsClock.getElapsedTime().asSeconds());
        }

        return true;
    }
};

} // namespace


////////////////////////////////////////////////////////////
int main()
{
    auto audioContext    = sf::AudioContext::create().value();
    auto graphicsContext = sf::GraphicsContext::create().value();

    Game game;

    if (!game.run())
        return 1;

    return 0;
}
