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
#include <SFML/Audio/Export.hpp>

#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/LifetimeDependee.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <optional>


namespace sf::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace sf::priv::MiniaudioUtils

namespace sf
{
class AudioContext;
class Listener;
class Sound;
class SoundStream;

////////////////////////////////////////////////////////////
class PlaybackDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<PlaybackDevice> createDefault(AudioContext& audioContext);

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit PlaybackDevice(AudioContext& audioContext, const PlaybackDeviceHandle& deviceHandle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~PlaybackDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice(PlaybackDevice&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice& operator=(PlaybackDevice&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    void transferResourcesTo(PlaybackDevice& other);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const PlaybackDeviceHandle& getDeviceHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateListener(const Listener& listener);

private:
    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    using ResourceEntryIndex = std::size_t;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] ResourceEntry
    {
        using InitFunc     = void (*)(void*);
        using TransferFunc = void (*)(void*, PlaybackDevice&, ResourceEntryIndex);

        void* resource{};

        InitFunc deinitializeFunc{};
        InitFunc reinitializeFunc{};

        TransferFunc transferFunc{};
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ResourceEntryIndex registerResource(void*                       resource,
                                                      ResourceEntry::InitFunc     deinitializeFunc,
                                                      ResourceEntry::InitFunc     reinitializeFunc,
                                                      ResourceEntry::TransferFunc transferFunc);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    void unregisterResource(ResourceEntryIndex resourceEntryIndex);

    [[nodiscard]] void* getMAEngine() const;


    struct Impl;
    priv::UniquePtr<Impl> m_impl;

    // TODO
    using SoundBase = priv::MiniaudioUtils::SoundBase;

    friend Listener;
    friend Sound;
    friend SoundBase;
    friend SoundStream;

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(AudioContext);
    SFML_DEFINE_LIFETIME_DEPENDEE(PlaybackDevice, SoundBase);
};

} // namespace sf
