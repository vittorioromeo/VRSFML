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
    enum class Positioning : unsigned char
    {
        Absolute = 0,
        Relative = 1,
    };

    Listener::Cone cone{radians(6.283185f), radians(6.283185f), 1.f};

    Vec3f position{0.f, 0.f, 0.f};   //!< Position where audio is heard from
    Vec3f direction{0.f, 0.f, -1.f}; //!< Forward-pointing vec3 from listener's perspective (not necessarily normalized)
    Vec3f velocity{0.f, 0.f, 0.f};   //!< Velocity in 3D space

    float volume{1.f}; //!< Volume (between `0` and `1`)
    float pitch{1.f};  //!< Pitch (between `0` and `+INF`, default is `1`)
    float pan{0.f};    //!< Pan (between `-1` and `1`)

    float attenuation{1.f};                  //!< Attenuation factor (between `0` and `+INF`, default is `1`)
    float directionalAttenuationFactor{1.f}; //!< Directional attenuation factor

    float dopplerFactor{1.f}; //!< Doppler factor

    float minDistance{1.f};                 //!< Minimum distance (between `0` and `+INF`, default is `1`)
    float maxDistance{SFML_BASE_FLOAT_MAX}; //!< Maximum distance (between `0` and `+INF`, default is `+INF`)

    float minGain{0.f}; //!< Minimum gain (between `0` and `1`, default is `0`)
    float maxGain{1.f}; //!< Maximum gain (between `0` and `1`, default is `1`)

    Positioning positioning{Positioning::Absolute}; //!< Positioning (absolute or relative to listener)

    bool playing{false}; //!< Whether the audio is currently playing
    bool looping{false}; //!< Whether the audio is looping

    bool spatializationEnabled{true}; //!< Whether spatialization is enabled
};

// TODO P0: also save cursor? or maybe have start,current,end? maybe rename to AudioState?

} // namespace sf
