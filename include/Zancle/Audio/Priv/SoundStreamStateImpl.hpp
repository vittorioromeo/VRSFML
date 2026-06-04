#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "ZancleBase/InPlacePImpl.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class ChannelMap;
class PlaybackDevice;
class Time;
} // namespace za

namespace za::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace za::priv::MiniaudioUtils


namespace za::priv
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
    bool (*onGetData)(void* state, zb::Vector<zb::I16>& outBuffer);
    void (*onSeek)(void* state, Time t);
    zb::Optional<zb::U64> (*onLoop)(void* state);
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
/// public audio API (`play`/`pause`/`setVolume`/...) either
/// inherit `MiniaudioSoundSource` themselves (see `za::Music`)
/// or use the convenience template `za::SoundStream<State>`.
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API SoundStreamStateImpl
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
    zb::InPlacePImpl<Internals, 2048> m_internals; //!< Holds miniaudio state (ma_sound etc.), hidden from this header
};

} // namespace za::priv
