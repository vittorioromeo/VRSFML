#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Listener.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/FloatMax.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct AudioSettings
{
    Listener::Cone cone{radians(6.283185f), radians(6.283185f), 1.f};

    Vec3f position{0.f, 0.f, 0.f};
    Vec3f direction{0.f, 0.f, -1.f};
    Vec3f velocity{0.f, 0.f, 0.f};
    float pitch{1.f};
    float pan{0.f};
    float volume{1.f};
    float directionalAttenuationFactor{1.f};
    float dopplerFactor{1.f};
    float minDistance{1.f};
    float maxDistance{SFML_BASE_FLOAT_MAX};
    float minGain{0.f};
    float maxGain{1.f};
    float rollOff{1.f};
    int   positioning{0 /* ma_positioning_absolute */}; // TODO P0: use enum class
    bool  playing{false};
    bool  looping{false};
    bool  spatializationEnabled{true};
    // TODO P0: also save cursor? or maybe have start,current,end?
    // TODO P0: maybe rename to AudioState?
};

} // namespace sf
