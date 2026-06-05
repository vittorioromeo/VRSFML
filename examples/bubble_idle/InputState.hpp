#pragma once

#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Container/Array.hpp"
#include "Zancle/Vocabulary/Optional.hpp"


namespace za
{
class Event;
}


////////////////////////////////////////////////////////////
struct InputState
{
    za::Array<za::Optional<za::Vec2i>, 32> fingerPositions{};

    za::Vec2i mousePosition;

    za::Array<bool, za::Keyboard::KeyCount> keys{};
    za::Array<bool, za::Mouse::ButtonCount> buttons{};

    bool alt{};
    bool control{};
    bool shift{};
    bool system{};

    ////////////////////////////////////////////////////////////
    void apply(const za::Event& event);
};
