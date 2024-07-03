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
#include "SFML/Audio/AudioContext.hpp"

#include <SFML/Audio/AudioDeviceHandle.hpp>
#include <SFML/Audio/Listener.hpp>

#include <SFML/System/InPlacePImpl.hpp>

#include <optional>
#include <vector>

#include <cstddef>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////

struct ma_engine;
struct ma_context;

namespace sf
{
class AudioContext;
class PlaybackDevice;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief High-level wrapper around the audio API, it manages
///        the creation and destruction of the audio device and
///        context and stores the device capabilities
///
////////////////////////////////////////////////////////////
class AudioDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioDevice(PlaybackDevice& playbackDevice, AudioContext& audioContext, const AudioDeviceHandle& deviceHandle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~AudioDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Get the audio engine
    ///
    /// \return The audio engine
    ///
    ////////////////////////////////////////////////////////////
    ma_engine& getEngine();

    void transferResourcesTo(AudioDevice& other);

    struct DeviceEntry
    {
        AudioDeviceHandle handle;
        bool              isDefault{};
    };

    using ResourceEntryIndex = std::size_t;

    struct ResourceEntry
    {
        using InitFunc     = void (*)(void*);
        using TransferFunc = void (*)(void*, PlaybackDevice&, ResourceEntryIndex);

        void* resource{};

        InitFunc deinitializeFunc{};
        InitFunc reinitializeFunc{};

        TransferFunc transferFunc{};
    };

    ////////////////////////////////////////////////////////////
    /// \brief Register an audio resource
    ///
    /// In order to support switching audio devices during
    /// runtime, all audio resources will have to be
    /// deinitialized using the old engine and device and then
    /// reinitialized using the new engine and device. In order
    /// for the AudioDevice to know which resources have to be
    /// notified, they need to register themselves with the
    /// AudioDevice using this function
    ///
    /// \param resource         A pointer uniquely identifying the object
    /// \param deinitializeFunc The function to call to deinitialize the object
    /// \param reinitializeFunc The function to call to reinitialize the object
    ///
    /// \see unregisterResource
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ResourceEntryIndex registerResource(void*                       resource,
                                                      ResourceEntry::InitFunc     deinitializeFunc,
                                                      ResourceEntry::InitFunc     reinitializeFunc,
                                                      ResourceEntry::TransferFunc transferFunc);

    ////////////////////////////////////////////////////////////
    /// \brief Unregister an audio resource
    ///
    /// \param resourceEntry The iterator returned when registering the resource
    ///
    /// \see registerResource
    ///
    ////////////////////////////////////////////////////////////
    void unregisterResource(ResourceEntryIndex resourceEntryIndex);

    ////////////////////////////////////////////////////////////
    /// \brief Change the global volume of all the sounds and musics
    ///
    /// The volume is a number between 0 and 100; it is combined with
    /// the individual volume of each sound / music.
    /// The default value for the volume is 100 (maximum).
    ///
    /// \param volume New global volume, in the range [0, 100]
    ///
    /// \see getGlobalVolume
    ///
    ////////////////////////////////////////////////////////////
    void setGlobalVolume(float volume);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current value of the global volume
    ///
    /// \return Current global volume, in the range [0, 100]
    ///
    /// \see setGlobalVolume
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getGlobalVolume() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the position of the listener in the scene
    ///
    /// The default listener's position is (0, 0, 0).
    ///
    /// \param position New listener's position
    ///
    /// \see getPosition, setDirection
    ///
    ////////////////////////////////////////////////////////////
    void setPosition(const Vector3f& position);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current position of the listener in the scene
    ///
    /// \return Listener's position
    ///
    /// \see setPosition
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f getPosition() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the forward vector of the listener in the scene
    ///
    /// The direction (also called "at vector") is the vector
    /// pointing forward from the listener's perspective. Together
    /// with the up vector, it defines the 3D orientation of the
    /// listener in the scene. The direction vector doesn't
    /// have to be normalized.
    /// The default listener's direction is (0, 0, -1).
    ///
    /// \param direction New listener's direction
    ///
    /// \see getDirection, setUpVector, setPosition
    ///
    ////////////////////////////////////////////////////////////
    void setDirection(const Vector3f& direction);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current forward vector of the listener in the scene
    ///
    /// \return Listener's forward vector (not normalized)
    ///
    /// \see setDirection
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f getDirection() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the velocity of the listener in the scene
    ///
    /// The default listener's velocity is (0, 0, -1).
    ///
    /// \param velocity New listener's velocity
    ///
    /// \see getVelocity, getDirection, setUpVector, setPosition
    ///
    ////////////////////////////////////////////////////////////
    void setVelocity(const Vector3f& velocity);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current forward vector of the listener in the scene
    ///
    /// \return Listener's velocity
    ///
    /// \see setVelocity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f getVelocity() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the cone properties of the listener in the audio scene
    ///
    /// The cone defines how directional attenuation is applied.
    /// The default cone of a sound is {2 * PI, 2 * PI, 1}.
    ///
    /// \param cone Cone properties of the listener in the scene
    ///
    /// \see getCone
    ///
    ////////////////////////////////////////////////////////////
    void setCone(const Listener::Cone& cone);

    ////////////////////////////////////////////////////////////
    /// \brief Get the cone properties of the listener in the audio scene
    ///
    /// \return Cone properties of the listener
    ///
    /// \see setCone
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Listener::Cone getCone() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the upward vector of the listener in the scene
    ///
    /// The up vector is the vector that points upward from the
    /// listener's perspective. Together with the direction, it
    /// defines the 3D orientation of the listener in the scene.
    /// The up vector doesn't have to be normalized.
    /// The default listener's up vector is (0, 1, 0). It is usually
    /// not necessary to change it, especially in 2D scenarios.
    ///
    /// \param upVector New listener's up vector
    ///
    /// \see getUpVector, setDirection, setPosition
    ///
    ////////////////////////////////////////////////////////////
    void setUpVector(const Vector3f& upVector);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current upward vector of the listener in the scene
    ///
    /// \return Listener's upward vector (not normalized)
    ///
    /// \see setUpVector
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f getUpVector() const;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Initialize the audio device and engine
    ///
    /// \return True if initialization was successful, false if it failed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool initialize(ma_context& context, const AudioDeviceHandle& selectedDeviceHandle);

public:
    ////////////////////////////////////////////////////////////
    /// \brief Structure holding listener properties
    ///
    ////////////////////////////////////////////////////////////
    struct ListenerProperties;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    priv::InPlacePImpl<Impl, 8192> m_impl; //!< Implementation details
};

} // namespace sf::priv
