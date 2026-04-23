#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"
#include "SFML/Audio/SoundStreamState.hpp"

#include "SFML/Base/Macros.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ChannelMap;
class PlaybackDevice;
class Time;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Public streamed audio source with the full audio API
///
/// `SoundStream<State>` is a concept-based audio stream that
/// drives a user-provided `State` from the audio thread while
/// exposing the usual `MiniaudioSoundSource` interface
/// (`play` / `pause` / `setVolume` / `setPitch` / ...). Use
/// it when you want to write a custom streamed source --
/// synthesized tone, network audio, procedural effect -- and
/// play it like a normal sound.
///
/// `State` is expected to expose:
///
/// \code
/// bool                      onGetData(base::Vector<base::I16>& outBuffer);
/// void                      onSeek(Time timeOffset);  // optional -- omit for generators that can't seek
/// base::Optional<base::U64> onLoop();                 // optional -- omit if the source never loops
/// \endcode
///
/// The destructor drains the audio thread before `State` is
/// destroyed, so the audio callback can never touch freed
/// memory. Miniaudio is an implementation detail and is not
/// exposed through this header.
///
/// Usage example (a trivial silent generator):
/// \code
/// struct MyState
/// {
///     bool onGetData(sf::base::Vector<sf::base::I16>& outBuffer)
///     {
///         outBuffer.resize(1024); // 1024 samples of silence
///         return true;            // keep streaming
///     }
/// };
///
/// sf::SoundStream<MyState> stream(playbackDevice,
///                                 sf::ChannelMap{sf::SoundChannel::Mono},
///                                 44'100u);
///
/// stream.setVolume(0.5f);
/// stream.play();
/// \endcode
///
/// Under the hood, `SoundStream` composes a
/// `sf::SoundStreamState<State>` (which holds the state and
/// the hidden miniaudio engine) and inherits
/// `MiniaudioSoundSource` for the public audio API. If you
/// want to hide `SoundStream.hpp` from your own public
/// header -- like `sf::Music` does -- inherit
/// `MiniaudioSoundSource` yourself and compose a
/// `SoundStreamState<State>` in your pImpl.
///
/// \see `sf::SoundStreamState`, `sf::Music`, `sf::Sound`
///
////////////////////////////////////////////////////////////
template <typename State>
class SoundStream : public priv::MiniaudioSoundSource
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a stream and initialize the state in place
    ///
    /// Extra arguments after `sampleRate` are forwarded to the
    /// `State` constructor.
    ///
    /// \param playbackDevice Playback device to render through (must outlive the stream)
    /// \param channelMap     Layout of audio channels in the produced sample frames
    /// \param sampleRate     Sample rate of the produced samples, in samples per second
    /// \param stateArgs      Arguments forwarded to the `State` constructor
    ///
    ////////////////////////////////////////////////////////////
    template <typename... StateArgs>
    explicit SoundStream(PlaybackDevice&    playbackDevice,
                         const ChannelMap&  channelMap,
                         const unsigned int sampleRate,
                         StateArgs&&... stateArgs) :
        m_state(playbackDevice, channelMap, sampleRate, SFML_BASE_FORWARD(stateArgs)...)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundStream() override = default;

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
    /// \brief Access the user-provided state
    ///
    /// \return Reference to the `State` held by this stream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] State& state()
    {
        return m_state.state();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Access the user-provided state (const overload)
    ///
    /// \return `const` reference to the `State` held by this stream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const State& state() const
    {
        return m_state.state();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the playback device this stream is rendered through
    ///
    /// \return Reference to the playback device this stream was constructed with
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] PlaybackDevice& getPlaybackDevice() const
    {
        return m_state.getPlaybackDevice();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the stream
    ///
    /// \param playingOffset New playing position, from the beginning of the stream
    ///
    ////////////////////////////////////////////////////////////
    void setPlayingOffset(const Time playingOffset) override
    {
        m_state.setPlayingOffset(playingOffset);
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Return the underlying `SoundBase`
    ///
    /// Routes the inherited `MiniaudioSoundSource` public API
    /// (volume, pitch, looping, ...) to the `ma_sound` owned
    /// by the internal `SoundStreamState<State>`, so there is
    /// one authoritative audio source per `SoundStream`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::MiniaudioUtils::SoundBase& getSoundBase() const override
    {
        return const_cast<SoundStream*>(this)->m_state.getSoundBase();
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SoundStreamState<State> m_state; //!< State + hidden miniaudio engine
};

} // namespace sf
