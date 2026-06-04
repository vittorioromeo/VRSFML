#pragma once

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ExampleUtils/Progress.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] HellPortal
{
    za::Vec2f position;
    Countdown life;
    zb::SizeT catIdx;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline za::Vec2f getDrawPosition() const
    {
        return position;
    }
};
