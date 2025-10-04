#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/DefaultShader.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/View.hpp" // used

#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"

#include "ExampleUtils.hpp"

#include <boost/pfr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>


////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
struct Settings
{
    float       simulationSpeed = 1.f;
    int         numParticles    = 100'000;
    std::string replayFilename  = "replay.bin";
    bool        enableRendering = true;
};


////////////////////////////////////////////////////////////
template <typename T>
struct IsStdString : std::false_type
{
};

template <>
struct IsStdString<std::string> : std::true_type
{
};


////////////////////////////////////////////////////////////
std::string pascalToHuman(const std::string& pascalCaseString)
{
    if (pascalCaseString.empty())
        return "";

    std::string humanReadable;
    humanReadable.reserve(pascalCaseString.length() + 5);

    humanReadable += std::toupper(static_cast<unsigned char>(pascalCaseString[0]));

    for (size_t i = 1; i < pascalCaseString.length(); ++i)
    {
        if (std::isupper(static_cast<unsigned char>(pascalCaseString[i])))
            humanReadable += ' ';

        humanReadable += pascalCaseString[i];
    }

    return humanReadable;
}


////////////////////////////////////////////////////////////
template <typename T>
void generateImguiEditor(const char* windowTitle, T& obj)
{
    static const auto names = boost::pfr::names_as_array<Settings>();

    ImGui::Begin(windowTitle);

    boost::pfr::for_each_field(obj,
                               [&](auto& field, size_t index)
    {
        const auto  nameStr = pascalToHuman(std::string{names[index]});
        const auto* name    = nameStr.c_str();

        using FieldType = std::decay_t<decltype(field)>;

        if constexpr (std::is_same_v<FieldType, bool>)
        {
            ImGui::Checkbox(name, &field);
        }
        else if constexpr (std::is_same_v<FieldType, int>)
        {
            ImGui::InputInt(name, &field);
        }
        else if constexpr (std::is_floating_point_v<FieldType>)
        {
            if constexpr (std::is_same_v<FieldType, float>)
            {
                ImGui::DragFloat(name, &field, 0.01f);
            }
            else // assume double
            {
                ImGui::InputDouble(name, &field, 0.01);
            }
        }
        else if constexpr (IsStdString<FieldType>::value)
        {
            char buffer[256];
            strncpy(buffer, field.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = 0;

            if (ImGui::InputText(name, buffer, sizeof(buffer)))
            {
                field = buffer;
            }
        }
        /* TODO: ...add more branches here... */
        else
        {
            ImGui::Text("%s: (Unsupported type)", name);
        }
    });

    ImGui::End();
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
    // Set up graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    //
    //
    // Set up window
    constexpr sf::Vec2f resolution{800.f, 600.f};

    auto window = makeDPIScaledRenderWindow({
        .size            = resolution.toVec2u(),
        .title           = "ImGui PFR",
        .fullscreen      = false,
        .resizable       = false,
        .closable        = false,
        .hasTitlebar     = false,
        .vsync           = false,
        .frametimeLimit  = 144u,
        .contextSettings = {.antiAliasingLevel = 16u},
    });

    //
    //
    // Set up imgui
    sf::ImGuiContext imGuiContext;

    //
    //
    // Load fonts
    ImFont* const fontImGuiGeistMono{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/geistmono.ttf", 32.f)};

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
            }
        }
        // ---

        ////////////////////////////////////////////////////////////
        // Update step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
        {
        }
        samplesUpdateMs.record(clock.getElapsedTime().asSeconds() * 1000.f);
        // ---

        ////////////////////////////////////////////////////////////
        // ImGui step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();

        constexpr float imguiMult = 1.f;

        const auto setFontScale = [&](const float x) { ImGui::SetWindowFontScale(x * imguiMult); };

        imGuiContext.update(window, fpsClock.getElapsedTime());

        ImGui::PushFont(fontImGuiGeistMono);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f * imguiMult); // Set corner radius

        static Settings settings;
        generateImguiEditor("Settings", settings);

        ImGui::PopStyleVar();
        ImGui::PopFont();
        // ---

        ////////////////////////////////////////////////////////////
        // Draw step
        ////////////////////////////////////////////////////////////
        // ---
        clock.restart();
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
