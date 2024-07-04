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
#include <SFML/System/InPlacePImpl.hpp>

#include <cstddef>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////

struct ma_engine;
struct ma_context;

namespace sf
{
class AudioContext;
class Listener;
class PlaybackDevice;
class PlaybackDeviceHandle;
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
    AudioDevice(PlaybackDevice& playbackDevice, AudioContext& audioContext, const PlaybackDeviceHandle& deviceHandle);

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

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    void transferResourcesTo(AudioDevice& other);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    using ResourceEntryIndex = std::size_t;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
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
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateFromListener(const Listener& listener);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    priv::InPlacePImpl<Impl, 8192> m_impl; //!< Implementation details
};

} // namespace sf::priv
