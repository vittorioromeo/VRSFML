#include "InputState.hpp"
#include "Zancle/Window/Event.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
void InputState::apply(const za::Event& event)
{
    if (const auto* e = event.getIf<za::Event::KeyPressed>())
    {
        keys[static_cast<zb::SizeT>(e->code)] = true;

        alt     = e->alt;
        control = e->control;
        shift   = e->shift;
        system  = e->system;

        return;
    }

    if (const auto* e = event.getIf<za::Event::KeyReleased>())
    {
        keys[static_cast<zb::SizeT>(e->code)] = false;

        alt     = e->alt;
        control = e->control;
        shift   = e->shift;
        system  = e->system;

        return;
    }

    if (const auto* e = event.getIf<za::Event::MouseButtonPressed>())
    {
        buttons[static_cast<zb::SizeT>(e->button)] = true;
        mousePosition                              = e->position;

        return;
    }

    if (const auto* e = event.getIf<za::Event::MouseButtonReleased>())
    {
        buttons[static_cast<zb::SizeT>(e->button)] = false;
        mousePosition                              = e->position;

        return;
    }

    if (const auto* e = event.getIf<za::Event::MouseMoved>())
    {
        mousePosition = e->position;

        return;
    }

    if (const auto* e = event.getIf<za::Event::TouchBegan>())
    {
        fingerPositions[e->finger].emplace(e->position);

        return;
    }

    if (const auto* e = event.getIf<za::Event::TouchMoved>())
    {
        fingerPositions[e->finger].emplace(e->position);

        return;
    }

    if (const auto* e = event.getIf<za::Event::TouchEnded>())
    {
        fingerPositions[e->finger].reset();

        return;
    }
}
