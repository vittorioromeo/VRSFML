#include "Zancle/ImGui/ImGuiContext.hpp"
#include "Zancle/ImGui/IncludeImGui.hpp"

#include "Zancle/Graphics/CircleShape.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "Zancle/System/Clock.hpp"

#include "ZancleBase/Optional.hpp"


int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    auto window = za::RenderWindow::create({
                                               .size  = {1280u, 720u},
                                               .title = "ImGui + Zancle = <3",
                                               .vsync = true,
                                           })
                      .value();

    // Share one font atlas across both contexts. The caller-owned atlas must be populated
    // before any ImGuiContext that references it is constructed. The main window's context
    // drives the atlas (per-frame updates); the child window's context just consumes it.
    // The driver also uploads the GL font texture; consumers adopt it.
    ImFontAtlas sharedFontAtlas;
    sharedFontAtlas.AddFontDefaultBitmap();

    za::ImGuiContext imGuiContext{za::ImGuiContext::createOwningAtlas(sharedFontAtlas)};

    auto childWindow = za::RenderWindow::create({
        .size  = {640u, 480u},
        .title = "ImGui-Zancle Child window",
        .vsync = true,
    });

    zb::Optional<za::ImGuiContext> childImGuiContext{za::ImGuiContext::createSharingAtlas(sharedFontAtlas)};

    za::Clock deltaClock;
    while (true)
    {
        // Main window event processing
        while (const zb::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        // Update
        const za::Time dt = deltaClock.restart();

        imGuiContext.update(window, dt);

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Main window drawing
        const za::CircleShape shape{{.fillColor = za::Color::Green, .radius = 100.f}};

        window.clear();
        window.draw(shape);
        imGuiContext.render(window);
        window.display();

        const auto processChildWindow = [&](za::RenderWindow& childWindowRef)
        {
            while (const zb::Optional event = childWindowRef.pollEvent())
            {
                childImGuiContext->processEvent(childWindowRef, *event);

                if (event->is<za::Event::Closed>())
                {
                    childImGuiContext.reset();
                    childWindow.reset();
                    return;
                }
            }

            childImGuiContext->update(childWindowRef, dt);

            ImGui::Begin("Works in a second window!");
            ImGui::Button("Example button");
            ImGui::End();

            const za::CircleShape shape2{{.fillColor = za::Color::Red, .radius = 50.f}};

            childWindowRef.clear();
            childWindowRef.draw(shape2);
            childImGuiContext->render(childWindowRef);
            childWindowRef.display();
        };

        // Child window event processing
        if (childWindow.hasValue())
            processChildWindow(*childWindow);
    }
}
