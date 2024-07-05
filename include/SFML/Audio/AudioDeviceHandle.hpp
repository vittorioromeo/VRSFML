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

#include <SFML/System/InPlacePImpl.hpp>
#include <SFML/System/PassKey.hpp>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class AudioContext;
class PlaybackDevice;
class CaptureDevice;
class PlaybackDeviceHandle;
class CaptureDeviceHandle;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
class [[nodiscard]] AudioDeviceHandle
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
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getName() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDefault() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    friend bool operator==(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    friend bool operator!=(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

protected:
    friend PlaybackDevice;
    friend PlaybackDeviceHandle;
    friend CaptureDevice;
    friend CaptureDeviceHandle;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const void* getMADeviceInfo() const;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(PassKey<AudioContext>&&, const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(PassKey<PlaybackDevice>&&, const void* maDeviceInfo);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioDeviceHandle(PassKey<CaptureDevice>&&, const void* maDeviceInfo);

private:
    struct Impl;
    InPlacePImpl<Impl, 2048> m_impl; //!< Implementation details
};

////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool operator==(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs);

////////////////////////////////////////////////////////////
/// \brief TODO
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

    [[nodiscard]] friend bool operator==(const StronglyTypedDeviceHandle& lhs, const StronglyTypedDeviceHandle& rhs)
    {
        return static_cast<const AudioDeviceHandle&>(lhs) == static_cast<const AudioDeviceHandle>(rhs);
    }

    [[nodiscard]] friend bool operator!=(const StronglyTypedDeviceHandle& lhs, const StronglyTypedDeviceHandle& rhs)
    {
        return !(lhs == rhs);
    }

private:
    using AudioDeviceHandle::getMADeviceInfo;
};

} // namespace sf::priv
