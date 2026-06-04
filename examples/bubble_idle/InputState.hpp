#pragma once

#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Array.hpp"
#include "ZancleBase/Optional.hpp"


namespace za
{
class Event;
}


////////////////////////////////////////////////////////////
struct InputState
{
    zb::Array<zb::Optional<za::Vec2i>, 32> fingerPositions{};

    za::Vec2i mousePosition;

    zb::Array<bool, za::Keyboard::KeyCount> keys{};
    zb::Array<bool, za::Mouse::ButtonCount> buttons{};

    bool alt{};
    bool control{};
    bool shift{};
    bool system{};

    ////////////////////////////////////////////////////////////
    void apply(const za::Event& event);
};
