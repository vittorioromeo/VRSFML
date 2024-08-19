#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/PassKey.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class AudioContext;
class AudioContextUtils;
class CaptureDevice;
class CaptureDeviceHandle;
class PlaybackDevice;
class PlaybackDeviceHandle;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_AUDIO_API AudioDeviceHandle
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~AudioDeviceHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioDeviceHandle(const AudioDeviceHandle& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioDeviceHandle(AudioDeviceHandle&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    AudioDeviceHandle& operator=(const AudioDeviceHandle& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    AudioDeviceHandle& operator=(AudioDeviceHandle&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the name of the device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getName() const;

    ////////////////////////////////////////////////////////////
    /// \brief Returns `true` if the device is a default one
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDefault() const;

    ////////////////////////////////////////////////////////////
    /// \brief Compare equality between device handles
    ///
    ////////////////////////////////////////////////////////////
    friend bool operator==(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Compare inequality between device handles
    ///
    ////////////////////////////////////////////////////////////
    friend bool operator!=(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

protected:
    friend PlaybackDevice;
    friend PlaybackDeviceHandle;
    friend CaptureDevice;
    friend CaptureDeviceHandle;

    ////////////////////////////////////////////////////////////
    /// \brief Create a device handle from miniaudio device info
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \brief Return a pointer to the stored miniaudio device info
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const void* getMADeviceInfo() const;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Create a device handle from miniaudio device info (passkey)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(base::PassKey<AudioContext>&&, const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Create a device handle from miniaudio device info (passkey)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(base::PassKey<AudioContextUtils>&&, const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Create a device handle from miniaudio device info (passkey)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(base::PassKey<PlaybackDevice>&&, const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Create a device handle from miniaudio device info (passkey)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(base::PassKey<CaptureDevice>&&, const void* maDeviceInfo);

private:
    struct Impl;
    base::InPlacePImpl<Impl, 2048> m_impl; //!< Implementation details
};

////////////////////////////////////////////////////////////
/// \brief Compare equality between device handles
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool operator==(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

////////////////////////////////////////////////////////////
/// \brief Compare inequality between device handles
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool operator!=(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

////////////////////////////////////////////////////////////
template <typename T>
class StronglyTypedDeviceHandle : private AudioDeviceHandle
{
    friend T;

    using AudioDeviceHandle::AudioDeviceHandle;

public:
    using AudioDeviceHandle::getName;
    using AudioDeviceHandle::isDefault;
    using AudioDeviceHandle::operator=;

    [[nodiscard, gnu::always_inline, gnu::pure]] friend bool operator==(const StronglyTypedDeviceHandle& lhs,
                                                                        const StronglyTypedDeviceHandle& rhs)
    {
        return static_cast<const AudioDeviceHandle&>(lhs) == static_cast<const AudioDeviceHandle>(rhs);
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] friend bool operator!=(const StronglyTypedDeviceHandle& lhs,
                                                                        const StronglyTypedDeviceHandle& rhs)
    {
        return !(lhs == rhs);
    }

private:
    using AudioDeviceHandle::getMADeviceInfo;
};

} // namespace sf::priv
