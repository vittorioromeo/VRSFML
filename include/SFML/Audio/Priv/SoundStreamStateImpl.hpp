#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ChannelMap;
class PlaybackDevice;
class Time;
} // namespace sf

namespace sf::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace sf::priv::MiniaudioUtils


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Function-pointer callbacks into a typed `State` owned by `SoundStreamState<State>`
///
/// The template wrapper provides these as captureless lambdas
/// converted to function pointers, so there is no heap
/// allocation and no virtual dispatch on the audio thread.
///
////////////////////////////////////////////////////////////
struct SoundStreamStateImplCallbacks
{
    bool (*onGetData)(void* state, base::Vector<base::I16>& outBuffer);
    void (*onSeek)(void* state, Time t);
    base::Optional<base::U64> (*onLoop)(void* state);
};


////////////////////////////////////////////////////////////
/// \brief Non-template audio-thread engine backing `SoundStreamState<State>`
///
/// Owns a miniaudio `ma_sound` and the scratch buffer that
/// the audio callback fills. Miniaudio is intentionally
/// hidden behind a pImpl so `SoundStream.hpp` can stay
/// miniaudio-free.
///
/// This class is a standalone value type; it does NOT inherit
/// from `MiniaudioSoundSource`. Wrapper classes that need the
/// public audio API (`play`/`pause`/`setVolume`/…) either
/// inherit `MiniaudioSoundSource` themselves (see `sf::Music`)
/// or use the convenience template `sf::SoundStream<State>`.
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundStreamStateImpl
{
public:
    ////////////////////////////////////////////////////////////
    explicit SoundStreamStateImpl(PlaybackDevice&               playbackDevice,
                                  const ChannelMap&             channelMap,
                                  unsigned int                  sampleRate,
                                  void*                         statePtr,
                                  SoundStreamStateImplCallbacks callbacks);

    ////////////////////////////////////////////////////////////
    ~SoundStreamStateImpl();

    ////////////////////////////////////////////////////////////
    SoundStreamStateImpl(const SoundStreamStateImpl&)            = delete;
    SoundStreamStateImpl& operator=(const SoundStreamStateImpl&) = delete;

    ////////////////////////////////////////////////////////////
    SoundStreamStateImpl(SoundStreamStateImpl&&)            = delete;
    SoundStreamStateImpl& operator=(SoundStreamStateImpl&&) = delete;

    ////////////////////////////////////////////////////////////
    void setPlayingOffset(Time playingOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Drain the audio thread and detach from the engine (idempotent)
    ///
    /// Called from `~SoundStreamState<State>` before `State` dies.
    /// Safe to call more than once.
    ///
    ////////////////////////////////////////////////////////////
    void detachFromEngineAndDrain();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] PlaybackDevice& getPlaybackDevice() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] MiniaudioUtils::SoundBase& getSoundBase();

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Internals;
    base::InPlacePImpl<Internals, 2048> m_internals; //!< Holds miniaudio state (ma_sound etc.), hidden from this header
};

} // namespace sf::priv
