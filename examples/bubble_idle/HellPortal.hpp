#pragma once

#include "ExampleUtils/Progress.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] HellPortal
{
    za::Vec2f position;
    Countdown life;
    za::SizeT catIdx;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline za::Vec2f getDrawPosition() const
    {
        return position;
    }
};
