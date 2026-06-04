#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "Zancle/Audio/SoundChannel.hpp"

#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/InitializerList.hpp" // IWYU pragma: keep
#include "ZancleBase/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Represents a specific arrangement of audio channels
///
/// A `ChannelMap` defines the layout of audio channels for a sound
/// or music stream. For example, a common channel map for stereo
/// sound would be `{SoundChannel::FrontLeft, SoundChannel::FrontRight}`.
///
/// Channel maps are used to specify how audio data is organized and
/// how it should be mapped to output speakers. They are essential for
/// ensuring that multi-channel audio is played back correctly.
///
/// The number of channels in a map is limited by `za::SoundChannelCount`.
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API ChannelMap
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates an empty channel map.
    ///
    ////////////////////////////////////////////////////////////
    ChannelMap() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an initializer list of sound channels
    ///
    /// This constructor allows for convenient creation of a ChannelMap
    /// using a list of `za::SoundChannel` enum values.
    ///
    /// Example:
    /// \code
    /// za::ChannelMap stereoMap = {za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};
    /// za::ChannelMap monoMap = {za::SoundChannel::Mono};
    /// \endcode
    ///
    /// If the number of channels in the initializer list exceeds
    /// `za::SoundChannelCount`, the behavior is undefined.
    ///
    /// \param channels An initializer list of `za::SoundChannel`
    ///
    ////////////////////////////////////////////////////////////
    ChannelMap(std::initializer_list<SoundChannel> channels);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of channels in the map
    ///
    /// \return The total number of sound channels in this map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check if the channel map is empty
    ///
    /// \return `true` if the channel map contains no channels, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isEmpty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get an iterator to the beginning of the channel map
    ///
    /// \return A pointer to the first `za::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SoundChannel* begin();

    ////////////////////////////////////////////////////////////
    /// \brief Get an iterator to the end of the channel map
    ///
    /// The returned iterator points to one past the last valid channel.
    ///
    /// \return A pointer to one past the last `za::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SoundChannel* end();

    ////////////////////////////////////////////////////////////
    /// \brief Get a const iterator to the beginning of the channel map
    ///
    /// \return A const pointer to the first `za::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundChannel* begin() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a const iterator to the end of the channel map
    ///
    /// The returned iterator points to one past the last valid channel.
    ///
    /// \return A const pointer to one past the last `za::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundChannel* end() const;

    ////////////////////////////////////////////////////////////
    /// \brief Access a channel at a specific index
    ///
    /// This function provides direct access to the sound channel
    /// at the given index. The behavior is undefined if `index`
    /// is out of bounds (i.e., greater than or equal to `getSize()`).
    ///
    /// \param index The index of the sound channel to access
    ///
    /// \return A const reference to the `za::SoundChannel` at the specified index
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundChannel& operator[](base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Append a sound channel to the map
    ///
    /// If adding the channel would exceed `za::SoundChannelCount`,
    /// the behavior is undefined. It's recommended to check the size
    /// or reserve capacity beforehand if necessary.
    ///
    /// \param channel The `za::SoundChannel` to add to the map
    ///
    ////////////////////////////////////////////////////////////
    void append(SoundChannel channel);

    ////////////////////////////////////////////////////////////
    /// \brief Check if this channel map is a permutation of another
    ///
    /// This function determines if the current ChannelMap contains the
    /// same set of sound channels as another ChannelMap, regardless of
    /// their order. For example, `{Mono}` is a permutation of `{Mono}`,
    /// and `{FrontLeft, FrontRight}` is a permutation of
    /// `{FrontRight, FrontLeft}`, but not of `{FrontLeft, Mono}`.
    ///
    /// Both channel maps must have the same number of channels for
    /// them to be considered permutations.
    ///
    /// \param rhs The other channel map to compare with
    ///
    /// \return `true` if this channel map is a permutation of `rhs`, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isPermutationOf(const ChannelMap& rhs) const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::InPlaceVector<SoundChannel, SoundChannelCount> m_channels;
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::ChannelMap
/// \ingroup audio
///
/// `za::ChannelMap` represents a specific arrangement of audio channels,
/// defining the layout for sound or music streams. It is crucial for
/// correct playback of multi-channel audio.
///
/// For example, to define a standard stereo layout:
/// \code
/// za::ChannelMap stereoMap = {za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};
/// \endcode
///
/// Or a 5.1 surround sound layout:
/// \code
/// za::ChannelMap surroundMap = {
///     za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight,
///     za::SoundChannel::FrontCenter,
///     za::SoundChannel::LowFrequencyEffects,
///     za::SoundChannel::RearLeft, za::SoundChannel::RearRight
/// };
/// \endcode
///
/// Channel maps are used by classes like `za::SoundBuffer` and `za::SoundStream`
/// to understand the structure of audio data.
///
/// \see za::SoundChannel, za::SoundBuffer, za::SoundStream
///
////////////////////////////////////////////////////////////
