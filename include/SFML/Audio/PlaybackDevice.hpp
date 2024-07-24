#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>

#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/LifetimeDependee.hpp>

#include <SFML/Base/Optional.hpp>
#include <SFML/Base/UniquePtr.hpp>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace sf::priv::MiniaudioUtils

namespace sf
{
class AudioContext;
class PlaybackDeviceHandle;
class Sound;
class SoundStream;
struct Listener;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
class PlaybackDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create the default playback device from `audioContext`
    ///
    /// \return Playback device on success, `sf::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<PlaybackDevice> createDefault(AudioContext& audioContext);

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
    /// \brief Transfer all active audio resources to `other`
    ///
    ////////////////////////////////////////////////////////////
    void transferResourcesTo(PlaybackDevice& other);

    ////////////////////////////////////////////////////////////
    /// \brief Get the device handle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const PlaybackDeviceHandle& getDeviceHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the playback device with `listener`'s parameters
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateListener(const Listener& listener);

private:
    // Friends
    using SoundBase = priv::MiniaudioUtils::SoundBase;
    friend SoundBase;

    ////////////////////////////////////////////////////////////
    /// \brief Internal representation of a resource entry handle
    ///
    ////////////////////////////////////////////////////////////
    using ResourceEntryIndex = unsigned int;

    ////////////////////////////////////////////////////////////
    /// \brief Tracks the lifetime of an audio resource and enables
    ///        transferring
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
    /// \brief Registers an audio resource with the playback device
    ///        and returns a handle to it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ResourceEntryIndex registerResource(void*                       resource,
                                                      ResourceEntry::InitFunc     deinitializeFunc,
                                                      ResourceEntry::InitFunc     reinitializeFunc,
                                                      ResourceEntry::TransferFunc transferFunc);

    ////////////////////////////////////////////////////////////
    /// \brief Unregisters a audio resource from the playback device
    ///
    ////////////////////////////////////////////////////////////
    void unregisterResource(ResourceEntryIndex resourceEntryIndex);

    ////////////////////////////////////////////////////////////
    /// \brief Gets the internal miniaudio engine pointer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] void* getMAEngine() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(AudioContext);
    SFML_DEFINE_LIFETIME_DEPENDEE(PlaybackDevice, SoundBase);
};

} // namespace sf
