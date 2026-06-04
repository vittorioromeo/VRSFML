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

#include "Zancle/System/Path.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToString.hpp"
#include "ZancleBase/Vector.hpp"


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
                          .title     = "SFML Raw Mouse Input",
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

    zb::Vector<zb::String> log;

    while (true)
    {
        while (const zb::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, windowView))
                continue;

            static const auto vec2ToString = [](const za::Vec2i vec2)
            { return '(' + zb::toString(vec2.x) + ", " + zb::toString(vec2.y) + ')'; };

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

        for (zb::SizeT i = 0u; i < log.size(); ++i)
        {
            mouseRawMovement.position = {50.f, static_cast<float>(i * 20) + 50.f};
            mouseRawMovement.setString(log[i]);
            window.draw(mouseRawMovement, {.view = windowView});
        }

        window.display();
    }
}
