#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup audio
/// \brief Types of sound channels that can be read/written from sound buffers/files
///
/// In multi-channel audio, each sound channel can be
/// assigned a position. The position of the channel is
/// used to determine where to place a sound when it
/// is spatialized. Assigning an incorrect sound channel
/// will result in multi-channel audio being positioned
/// incorrectly when using spatialization.
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] SoundChannel : unsigned char
{
    Unspecified,
    Mono,
    FrontLeft,
    FrontRight,
    FrontCenter,
    FrontLeftOfCenter,
    FrontRightOfCenter,
    LowFrequencyEffects,
    BackLeft,
    BackRight,
    BackCenter,
    SideLeft,
    SideRight,
    TopCenter,
    TopFrontLeft,
    TopFrontRight,
    TopFrontCenter,
    TopBackLeft,
    TopBackRight,
    TopBackCenter
};

////////////////////////////////////////////////////////////
enum : unsigned int
{
    /// Total number of possible sound channels, including unspecified
    SoundChannelCount = static_cast<unsigned int>(SoundChannel::TopBackCenter) + 1u
};

} // namespace sf
