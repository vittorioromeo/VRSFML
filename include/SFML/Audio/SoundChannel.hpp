#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
/// The set of values mirrors the standard speaker positions
/// recognized by miniaudio (and most other audio backends).
///
/// \see `sf::ChannelMap`
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] SoundChannel : unsigned char
{
    Unspecified,         //!< Channel position is not known or not relevant
    Mono,                //!< Single mono channel (no positional meaning)
    FrontLeft,           //!< Front-left speaker (stereo / surround layouts)
    FrontRight,          //!< Front-right speaker (stereo / surround layouts)
    FrontCenter,         //!< Front-center speaker (e.g. dialog channel in surround)
    FrontLeftOfCenter,   //!< Front speaker located between front-left and front-center
    FrontRightOfCenter,  //!< Front speaker located between front-center and front-right
    LowFrequencyEffects, //!< Low-frequency effects channel (the ".1" of 5.1 / 7.1 layouts)
    BackLeft,            //!< Back-left speaker
    BackRight,           //!< Back-right speaker
    BackCenter,          //!< Back-center speaker
    SideLeft,            //!< Side-left speaker (between front-left and back-left)
    SideRight,           //!< Side-right speaker (between front-right and back-right)
    TopCenter,           //!< Top-center speaker (height channel directly above the listener)
    TopFrontLeft,        //!< Top-front-left speaker (height channel)
    TopFrontRight,       //!< Top-front-right speaker (height channel)
    TopFrontCenter,      //!< Top-front-center speaker (height channel)
    TopBackLeft,         //!< Top-back-left speaker (height channel)
    TopBackRight,        //!< Top-back-right speaker (height channel)
    TopBackCenter        //!< Top-back-center speaker (height channel)
};

////////////////////////////////////////////////////////////
enum : unsigned int
{
    /// Total number of distinct values in `sf::SoundChannel`, including `Unspecified`.
    /// Useful as a fixed upper bound when storing per-channel data.
    SoundChannelCount = static_cast<unsigned int>(SoundChannel::TopBackCenter) + 1u
};

} // namespace sf
