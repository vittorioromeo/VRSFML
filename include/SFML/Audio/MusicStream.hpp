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

#include <SFML/Audio/SoundStream.hpp>

#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <optional>

#include <cstdint>


namespace sf
{
class Time;
class PlaybackDevice;
class MusicSource;

class MusicStream : public SoundStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Structure defining a time range using the template type
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    struct [[nodiscard]] Span
    {
        T offset{}; //!< The beginning offset of the time range
        T length{}; //!< The length of the time range
    };

    // Define the relevant Span types
    using TimeSpan = Span<Time>;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    explicit MusicStream(PlaybackDevice& playbackDevice, MusicSource& music);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~MusicStream() override;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    MusicStream(MusicStream&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    MusicStream& operator=(MusicStream&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the positions of the of the sound's looping sequence
    ///
    /// \return Loop Time position class.
    ///
    /// \warning Since setLoopPoints() performs some adjustments on the
    /// provided values and rounds them to internal samples, a call to
    /// getLoopPoints() is not guaranteed to return the same times passed
    /// into a previous call to setLoopPoints(). However, it is guaranteed
    /// to return times that will map to the valid internal samples of
    /// this music stream if they are later passed to setLoopPoints().
    ///
    /// \see setLoopPoints
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TimeSpan getLoopPoints() const;

    ////////////////////////////////////////////////////////////
    /// \brief Sets the beginning and duration of the sound's looping sequence using sf::Time
    ///
    /// setLoopPoints() allows for specifying the beginning offset and the duration of the loop such that, when the music
    /// is enabled for looping, it will seamlessly seek to the beginning whenever it
    /// encounters the end of the duration. Valid ranges for timePoints.offset and timePoints.length are
    /// [0, Dur) and (0, Dur-offset] respectively, where Dur is the value returned by getDuration().
    /// Note that the EOF "loop point" from the end to the beginning of the stream is still honored,
    /// in case the caller seeks to a point after the end of the loop range. This function can be
    /// safely called at any point after a stream is opened, and will be applied to a playing sound
    /// without affecting the current playing offset.
    ///
    /// \warning Setting the loop points while the stream's status is Paused
    /// will set its status to Stopped. The playing offset will be unaffected.
    ///
    /// \param timePoints The definition of the loop. Can be any time points within the sound's length
    ///
    /// \see getLoopPoints
    ///
    ////////////////////////////////////////////////////////////
    void setLoopPoints(TimeSpan timePoints);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Request a new chunk of audio samples from the stream source
    ///
    /// This function fills the chunk from the next samples
    /// to read from the audio file.
    ///
    /// \param data Chunk of data to fill
    ///
    /// \return True to continue playback, false to stop
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onGetData(Chunk& data) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source
    ///
    /// \param timeOffset New playing position, from the beginning of the music
    ///
    ////////////////////////////////////////////////////////////
    void onSeek(Time timeOffset) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source to the loop offset
    ///
    /// This is called by the underlying SoundStream whenever it needs us to reset
    /// the seek position for a loop. We then determine whether we are looping on a
    /// loop point or the end-of-file, perform the seek, and return the new position.
    ///
    /// \return The seek position after looping (or std::nullopt if there's no loop)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::optional<std::uint64_t> onLoop() override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    priv::UniquePtr<Impl> m_impl;

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(Music);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::MusicStream
/// \ingroup audio
///
/// TODO
///
/// \see sf::MusicSource, sf::SoundStream
///
////////////////////////////////////////////////////////////
