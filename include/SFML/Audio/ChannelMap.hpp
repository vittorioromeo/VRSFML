#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Represents a specific arrangement of audio channels
///
/// A ChannelMap defines the layout of audio channels for a sound
/// or music stream. For example, a common channel map for stereo
/// sound would be `{SoundChannel::FrontLeft, SoundChannel::FrontRight}`.
///
/// ChannelMaps are used to specify how audio data is organized and
/// how it should be mapped to output speakers. They are essential for
/// ensuring that multi-channel audio is played back correctly.
///
/// The number of channels in a map is limited by `sf::SoundChannelCount`.
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API ChannelMap
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
    /// using a list of `sf::SoundChannel` enum values.
    ///
    /// Example:
    /// \code
    /// sf::ChannelMap stereoMap = {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};
    /// sf::ChannelMap monoMap = {sf::SoundChannel::Mono};
    /// \endcode
    ///
    /// If the number of channels in the initializer list exceeds
    /// `sf::SoundChannelCount`, the behavior is undefined.
    ///
    /// \param channels An initializer list of `sf::SoundChannel`
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
    /// \return True if the channel map contains no channels, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isEmpty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get an iterator to the beginning of the channel map
    ///
    /// \return A pointer to the first `sf::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SoundChannel* begin();

    ////////////////////////////////////////////////////////////
    /// \brief Get an iterator to the end of the channel map
    ///
    /// The returned iterator points to one past the last valid channel.
    ///
    /// \return A pointer to one past the last `sf::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SoundChannel* end();

    ////////////////////////////////////////////////////////////
    /// \brief Get a const iterator to the beginning of the channel map
    ///
    /// \return A const pointer to the first `sf::SoundChannel` in the map
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundChannel* begin() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a const iterator to the end of the channel map
    ///
    /// The returned iterator points to one past the last valid channel.
    ///
    /// \return A const pointer to one past the last `sf::SoundChannel` in the map
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
    /// \return A const reference to the `sf::SoundChannel` at the specified index
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundChannel& operator[](base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Reserve storage for a specific number of channels
    ///
    /// This function allocates memory to accommodate at least `count`
    /// channels. If `count` is less than or equal to the current
    /// capacity, this function does nothing.
    ///
    /// This can be useful to avoid reallocations if you know
    /// in advance how many channels you will be adding.
    ///
    /// If `count` exceeds `sf::SoundChannelCount`, the actual
    /// capacity will be limited by `sf::SoundChannelCount`.
    ///
    /// \param count The minimum number of channels to reserve storage for
    ///
    ////////////////////////////////////////////////////////////
    void reserve(base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief Append a sound channel to the map
    ///
    /// If adding the channel would exceed `sf::SoundChannelCount`,
    /// the behavior is undefined. It's recommended to check the size
    /// or reserve capacity beforehand if necessary.
    ///
    /// \param channel The `sf::SoundChannel` to add to the map
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
    /// \param rhs The other ChannelMap to compare with
    ///
    /// \return True if this channel map is a permutation of `rhs`, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isPermutationOf(const ChannelMap& rhs) const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::InPlaceVector<SoundChannel, SoundChannelCount> m_channels{};
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::ChannelMap
/// \ingroup audio
///
/// sf::ChannelMap represents a specific arrangement of audio channels,
/// defining the layout for sound or music streams. It is crucial for
/// correct playback of multi-channel audio.
///
/// For example, to define a standard stereo layout:
/// \code
/// sf::ChannelMap stereoMap = {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};
/// \endcode
///
/// Or a 5.1 surround sound layout:
/// \code
/// sf::ChannelMap surroundMap = {
///     sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight,
///     sf::SoundChannel::FrontCenter,
///     sf::SoundChannel::LowFrequencyEffects,
///     sf::SoundChannel::RearLeft, sf::SoundChannel::RearRight
/// };
/// \endcode
///
/// ChannelMaps are used by classes like `sf::SoundBuffer` and `sf::SoundStream`
/// to understand the structure of audio data.
///
/// \see sf::SoundChannel, sf::SoundBuffer, sf::SoundStream
///
////////////////////////////////////////////////////////////
