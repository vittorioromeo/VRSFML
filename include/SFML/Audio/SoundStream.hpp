#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ChannelMap;
class EffectProcessor;
class PlaybackDevice;
class Time;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for streamed audio sources
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundStream : public priv::MiniaudioSoundSource
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Structure defining a chunk of audio data to stream
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Chunk
    {
        const base::I16* samples{};     //!< Pointer to the audio samples
        base::SizeT      sampleCount{}; //!< Number of samples pointed by Samples
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundStream() override;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundStream(const SoundStream&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundStream& operator=(const SoundStream&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundStream(SoundStream&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundStream& operator=(SoundStream&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the stream
    ///
    /// The playing position can be changed when the stream is
    /// either paused or playing. Changing the playing position
    /// when the stream is stopped has no effect, since playing
    /// the stream would reset its position.
    ///
    /// \param playingOffset New playing position, from the beginning of the stream
    ///
    /// \see `getPlayingOffset`
    ///
    ////////////////////////////////////////////////////////////
    void setPlayingOffset(Time playingOffset) override;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor is only meant to be called by derived classes.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SoundStream(PlaybackDevice& playbackDevice, const ChannelMap& channelMap, unsigned int sampleRate);

    ////////////////////////////////////////////////////////////
    /// \brief Request a new chunk of audio samples from the stream source
    ///
    /// This function must be overridden by derived classes to provide
    /// the audio samples to play. It is called continuously by the
    /// streaming loop, in a separate thread.
    /// The source can choose to stop the streaming loop at any time, by
    /// returning `false` to the caller.
    /// If you return `true` (i.e. continue streaming) it is important that
    /// the returned array of samples is not empty; this would stop the stream
    /// due to an internal limitation.
    ///
    /// \param data Chunk of data to fill
    ///
    /// \return `true` to continue playback, `false` to stop
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool onGetData(Chunk& data) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source
    ///
    /// This function must be overridden by derived classes to
    /// allow random seeking into the stream source.
    ///
    /// \param timeOffset New playing position, relative to the beginning of the stream
    ///
    ////////////////////////////////////////////////////////////
    virtual void onSeek(Time timeOffset) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source to the beginning of the loop
    ///
    /// This function can be overridden by derived classes to
    /// allow implementation of custom loop points. Otherwise,
    /// it just calls `onSeek(Time{})` and returns 0.
    ///
    /// \return The seek position after looping (or `base::nullOpt` if there's no loop)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<base::U64> onLoop();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get the sound object
    ///
    /// \return The sound object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::MiniaudioUtils::SoundBase& getSoundBase() const override;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 2560> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundStream
/// \ingroup audio
///
/// Unlike audio buffers (see `sf::SoundBuffer`), audio streams
/// are never completely loaded in memory. Instead, the audio
/// data is acquired continuously while the stream is playing.
/// This behavior allows to play a sound with no loading delay,
/// and keeps the memory consumption very low.
///
/// Sound sources that need to be streamed are usually big files
/// (compressed audio musics that would eat hundreds of MB in memory)
/// or files that would take a lot of time to be received
/// (sounds played over the network).
///
/// `sf::SoundStream` is a base class that doesn't care about the
/// stream source, which is left to the derived class. SFML provides
/// a built-in specialization for big files (see `sf::Music`).
/// No network stream source is provided, but you can write your own
/// by combining this class with the network module.
///
/// A derived class has to override two virtual functions:
/// \li `onGetData` fills a new chunk of audio data to be played
/// \li `onSeek` changes the current playing position in the source
///
/// It is important to note that each SoundStream is played in its
/// own separate thread, so that the streaming loop doesn't block the
/// rest of the program. In particular, the `onGetData` and `onSeek`
/// virtual functions may sometimes be called from this separate thread.
/// It is important to keep this in mind, because you may have to take
/// care of synchronization issues if you share data between threads.
///
/// Usage example:
/// \code
/// class CustomStream : public sf::SoundStream
/// {
/// public:
///
///     [[nodiscard]] bool open(const std::string& location)
///     {
///         // Open the source and get audio settings
///         ...
///         unsigned int channelCount = 2; // Stereo
///         unsigned int sampleRate = 44100; // 44100 Hz
///
///         // Initialize the stream -- important!
///         initialize(channelCount, sampleRate, {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
///         return true;
///     }
///
/// private:
///
///     bool onGetData(Chunk& data) override
///     {
///         // Fill the chunk with audio data from the stream source
///         // (note: must not be empty if you want to continue playing)
///         data.samples = ...;
///
///         // Return true to continue playing
///         data.sampleCount = ...;
///         return true;
///     }
///
///     void onSeek(sf::Time timeOffset) override
///     {
///         // Change the current position in the stream source
///         ...
///     }
/// };
///
/// // Usage
/// CustomStream stream;
/// stream.open("path/to/stream");
/// stream.play();
/// \endcode
///
/// \see `sf::Music`
///
////////////////////////////////////////////////////////////
