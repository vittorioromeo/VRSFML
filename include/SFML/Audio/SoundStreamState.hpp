#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Priv/SoundStreamStateImpl.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ChannelMap;
class PlaybackDevice;
} // namespace sf

namespace sf::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace sf::priv::MiniaudioUtils


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Value-typed, concept-based audio stream core
///
/// `SoundStreamState<State>` owns a user-provided `State` and
/// drives it from the audio thread. The state is expected to
/// expose:
///
/// \code
/// bool                      onGetData(base::Vector<base::I16>& outBuffer);
/// void                      onSeek(Time timeOffset);  // optional -- omit for generators that can't seek
/// base::Optional<base::U64> onLoop();                 // optional -- omit if the source never loops
/// \endcode
///
/// `onSeek` and `onLoop` are detected via `requires` and
/// skipped when absent: if `State` doesn't define `onSeek`,
/// seeking is a no-op on the state; if `State` doesn't define
/// `onLoop`, reaching EOF simply stops playback.
///
/// The destructor drains the audio thread before `State` is
/// destroyed, so there is no window in which the audio
/// callback can touch freed memory. All miniaudio interaction
/// lives inside the non-template
/// `priv::SoundStreamStateImpl`, so this header does not pull
/// in `<miniaudio.h>`.
///
/// This class is standalone; it does NOT inherit from
/// `MiniaudioSoundSource`, so it does not expose
/// `play`/`pause`/`setVolume`/... To get those, either wrap it
/// with `sf::SoundStream<State>` (the public user-facing
/// template) or inherit `MiniaudioSoundSource` yourself and
/// compose a `SoundStreamState` (what `sf::Music` does, to
/// keep `SoundStream.hpp` out of its public header).
///
/// \see `sf::SoundStream`, `sf::Music`
///
////////////////////////////////////////////////////////////
template <typename State>
class SoundStreamState
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a stream core and initialize the state in place
    ///
    /// Extra arguments after `sampleRate` are forwarded to the
    /// `State` constructor. This lets `State` contain
    /// non-movable members (mutexes, atomics) without requiring
    /// the caller to materialize a `State` value first.
    ///
    /// \param playbackDevice Playback device to render through (must outlive the stream)
    /// \param channelMap     Layout of audio channels in the produced sample frames
    /// \param sampleRate     Sample rate of the produced samples, in samples per second
    /// \param stateArgs      Arguments forwarded to the `State` constructor
    ///
    ////////////////////////////////////////////////////////////
    template <typename... StateArgs>
    explicit SoundStreamState(PlaybackDevice&    playbackDevice,
                              const ChannelMap&  channelMap,
                              const unsigned int sampleRate,
                              StateArgs&&... stateArgs) :
        // NOTE: `&m_state` is passed to `m_impl` before `m_state` is
        // constructed. That is safe: miniaudio does not invoke any
        // callback before `play()` is called, and by that point
        // `m_state` is fully constructed.
        m_impl(playbackDevice, channelMap, sampleRate, &m_state, makeCallbacks()),
        m_state(SFML_BASE_FORWARD(stateArgs)...)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Drains the audio thread before `m_state` dies. This is
    /// the only place the safety invariant is enforced; no user
    /// cooperation is required.
    ///
    ////////////////////////////////////////////////////////////
    ~SoundStreamState()
    {
        // Drain audio thread BEFORE `m_state` dies. `m_impl`'s own
        // destructor also drains (idempotently), but by the time it
        // runs `m_state` has already been destroyed.
        m_impl.detachFromEngineAndDrain();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundStreamState(const SoundStreamState&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundStreamState& operator=(const SoundStreamState&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundStreamState(SoundStreamState&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundStreamState& operator=(SoundStreamState&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Access the user-provided state
    ///
    /// \return Reference to the `State` held by this stream core
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] State& state()
    {
        return m_state;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Access the user-provided state (const overload)
    ///
    /// \return `const` reference to the `State` held by this stream core
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const State& state() const
    {
        return m_state;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the playback device this stream core is rendered through
    ///
    /// \return Reference to the playback device this stream was constructed with
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] PlaybackDevice& getPlaybackDevice() const
    {
        return m_impl.getPlaybackDevice();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the underlying miniaudio `SoundBase`
    ///
    /// Meant for wrapper classes that independently inherit
    /// `MiniaudioSoundSource` (e.g. `sf::Music`) and need to
    /// route their `getSoundBase()` override through this
    /// stream core so the outer audio API operates on the same
    /// `ma_sound`. Not intended for end-user code.
    ///
    /// \return Reference to the miniaudio sound base held by this stream core
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::MiniaudioUtils::SoundBase& getSoundBase()
    {
        return m_impl.getSoundBase();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the stream
    ///
    /// Resets the internal streaming state, updates the sample
    /// position, and calls `state.onSeek(...)` if `State`
    /// provides it.
    ///
    /// \param playingOffset New playing position, from the beginning of the stream
    ///
    ////////////////////////////////////////////////////////////
    void setPlayingOffset(const Time playingOffset)
    {
        m_impl.setPlayingOffset(playingOffset);
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Build the function-pointer callbacks for `SoundStreamStateImpl`
    ///
    /// Captureless lambdas convert to function pointers, so this
    /// compiles to a table of three raw function pointers with no
    /// heap, no virtual dispatch, no overhead on the audio thread.
    /// The `if constexpr (requires { ... })` guards make
    /// `onSeek` / `onLoop` optional on the `State` type.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static priv::SoundStreamStateImplCallbacks makeCallbacks()
    {
        return {
            .onGetData = [](void* const s, base::Vector<base::I16>& outBuffer) -> bool
        { return static_cast<State*>(s)->onGetData(outBuffer); },

            .onSeek = [](void* const s, const Time t) -> void
        {
            if constexpr (requires { static_cast<State*>(s)->onSeek(t); })
                static_cast<State*>(s)->onSeek(t);
        },

            .onLoop = [](void* const s) -> base::Optional<base::U64>
        {
            if constexpr (requires { static_cast<State*>(s)->onLoop(); })
                return static_cast<State*>(s)->onLoop();
            else
                return base::nullOpt;
        },
        };
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    priv::SoundStreamStateImpl m_impl;  //!< Non-template engine (holds miniaudio state inside its pImpl)
    State                      m_state; //!< User-provided state invoked from the audio thread
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundStreamState
/// \ingroup audio
///
/// `sf::SoundStreamState<State>` is the low-level primitive
/// for producing audio on demand: unlike `sf::SoundBuffer`,
/// the samples are generated or fetched continuously while
/// the stream is playing, so it is ideal for long tracks or
/// data that isn't available up front (network audio,
/// synthesized tones, decoded music files, etc.).
///
/// The `State` type provides the behavior -- it is invoked
/// from the audio thread via three method hooks
/// (`onGetData`, and optionally `onSeek` / `onLoop`). Because
/// `SoundStreamState` is a concrete, non-inheriting template,
/// there is no derived-class member-ordering hazard: the
/// destructor always drains the audio thread before `State`
/// is destroyed.
///
/// `SoundStreamState` is primarily an implementation detail
/// used by `sf::SoundStream<State>` (the public user-facing
/// template that adds the `play`/`pause`/`setVolume` API) and
/// by `sf::Music` (which composes it via pImpl). Most users
/// will want `sf::SoundStream<State>` directly.
///
/// \see `sf::SoundStream`, `sf::Music`, `sf::Sound`
///
////////////////////////////////////////////////////////////
