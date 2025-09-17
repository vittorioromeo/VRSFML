#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

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
    // Handle empty string case
    if (pascalCaseString.empty())
    {
        return "";
    }

    std::string humanReadable;
    // Reserve memory to avoid reallocations, adding a little extra for spaces
    humanReadable.reserve(pascalCaseString.length() + 5);

    // 1. Capitalize the first letter
    humanReadable += std::toupper(static_cast<unsigned char>(pascalCaseString[0]));

    // 2. Iterate through the rest of the string
    for (size_t i = 1; i < pascalCaseString.length(); ++i)
    {
        // 3. If a character is uppercase, prepend a space
        if (std::isupper(static_cast<unsigned char>(pascalCaseString[i])))
        {
            humanReadable += ' ';
        }
        // 4. Append the character itself
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

    // Use Boost.PFR to iterate over each field of the struct.
    boost::pfr::for_each_field(obj,
                               [&](auto& field, size_t index)
    {
        // Get the name for the current field from our names array.
        const auto  nameStr = pascalToHuman(std::string{names[index]});
        const auto* name    = nameStr.c_str();

        // Use `if constexpr` to generate the correct ImGui widget based on the field's type.
        // `std::decay_t` removes references and const qualifiers to get the underlying type.
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
            // Use DragFloat for a nice user experience.
            if constexpr (std::is_same_v<FieldType, float>)
            {
                ImGui::DragFloat(name, &field, 0.01f);
            }
            else
            { // double
                ImGui::InputDouble(name, &field, 0.01);
            }
        }
        else if constexpr (IsStdString<FieldType>::value)
        {
            // ImGui's InputText works with char buffers, not std::string directly.
            // We need a small wrapper.
            // Note: For simplicity, using a fixed-size buffer. For very long strings,
            // you might need a dynamic buffer.
            char buffer[256];
            strncpy(buffer, field.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = 0; // Ensure null termination

            if (ImGui::InputText(name, buffer, sizeof(buffer)))
            {
                field = buffer; // Update the string if the user made a change
            }
        }
        // --- EXTENSIBILITY POINT ---
        // You can add more `else if constexpr` blocks here to handle custom types.
        // For example, an ImVec4 for a color picker.
        // else if constexpr (std::is_same_v<FieldType, ImVec4>) {
        //     ImGui::ColorEdit4(name, &field.x);
        // }
        else
        {
            // Fallback for any unsupported types.
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
