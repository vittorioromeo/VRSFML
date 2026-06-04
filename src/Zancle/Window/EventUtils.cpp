// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/Keyboard.hpp"


namespace za::EventUtils
{
////////////////////////////////////////////////////////////
bool isClosedOrEscapeKeyPressed(const Event& event)
{
    return event.is<za::Event::Closed>() ||
           (event.is<za::Event::KeyPressed>() && event.getIf<za::Event::KeyPressed>()->code == za::Keyboard::Key::Escape);
}

} // namespace za::EventUtils
