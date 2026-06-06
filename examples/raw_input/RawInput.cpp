////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ExampleUtils/Scaling.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Text.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/String/ToString.hpp"
#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = za::GraphicsContext::create().value();

    // Create the main window
    constexpr za::Vec2f windowSize{800.f, 600.f};

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = windowSize.toVec2u(),
                          .title     = "Zancle Raw Mouse Input",
                          .resizable = true,
                          .vsync     = true,
                      })
                      .value();

    auto windowView = computeAspectRatioAwareView(window.getSize().toVec2f(), windowSize);

    // Open the application font and pass it to the Effect class
    const auto font = za::Font::openFromFile("resources/tuffy.ttf").value();

    // Create the mouse position and mouse raw movement texts
    za::Text mousePosition(font,
                           {
                               .position      = {400.f, 300.f},
                               .string        = "",
                               .characterSize = 20u,
                               .fillColor     = za::Color::White,
                           });

    za::Text mouseRawMovement(font,
                              {
                                  .string        = "",
                                  .characterSize = 20u,
                                  .fillColor     = za::Color::White,
                              });

    za::Vector<za::String> log;

    while (true)
    {
        while (const za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, windowView))
                continue;

            static const auto vec2ToString = [](const za::Vec2i vec2)
            { return '(' + za::toString(vec2.x) + ", " + za::toString(vec2.y) + ')'; };

            if (const auto* const mouseMoved = event->getIf<za::Event::MouseMoved>())
                mousePosition.setString("Mouse Position: " + vec2ToString(mouseMoved->position));

            if (const auto* const mouseMovedRaw = event->getIf<za::Event::MouseMovedRaw>())
            {
                log.emplaceBack("Mouse Movement: " + vec2ToString(mouseMovedRaw->delta));

                if (log.size() > 24u)
                    log.erase(log.begin());
            }
        }

        window.clear();
        window.draw(mousePosition, {.view = windowView});

        for (za::SizeT i = 0u; i < log.size(); ++i)
        {
            mouseRawMovement.position = {50.f, static_cast<float>(i * 20) + 50.f};
            mouseRawMovement.setString(log[i]);
            window.draw(mouseRawMovement, {.view = windowView});
        }

        window.display();
    }
}
