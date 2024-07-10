////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Listener.hpp>

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector3.hpp>

#include <cfloat>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ma_sound;


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SavedSettings
{
    float          pitch{1.f};
    float          pan{0.f};
    float          volume{1.f};
    bool           spatializationEnabled{true};
    Vector3f       position{0.f, 0.f, 0.f};
    Vector3f       direction{0.f, 0.f, -1.f};
    float          directionalAttenuationFactor{1.f};
    Vector3f       velocity{0.f, 0.f, 0.f};
    float          dopplerFactor{1.f};
    int            positioning{0 /* ma_positioning_absolute */};
    float          minDistance{1.f};
    float          maxDistance{FLT_MAX};
    float          minGain{0.f};
    float          maxGain{1.f};
    float          rollOff{1.f};
    bool           playing{false};
    bool           looping{false};
    Listener::Cone cone{radians(6.283185f), radians(6.283185f), 0.f};

    ////////////////////////////////////////////////////////////
    explicit SavedSettings() = default;

    ////////////////////////////////////////////////////////////
    explicit SavedSettings(const ma_sound& sound);

    ////////////////////////////////////////////////////////////
    void applyOnto(ma_sound& sound) const;
};

} // namespace sf::priv
