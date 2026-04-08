#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class SoundBuffer;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized SoundRecorder which stores the captured
///        audio data into a sound buffer
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundBufferRecorder : public SoundRecorder
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SoundBufferRecorder();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundBufferRecorder() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sound buffer containing the captured audio data
    ///
    /// The sound buffer is built when the capture stops, so
    /// this function must only be called after at least one
    /// successful `start()`/`stop()` cycle on this recorder
    /// (calling it before any data has been captured triggers
    /// an assertion). The returned reference is read-only;
    /// copy the buffer if you need to modify it.
    ///
    /// \return Read-only reference to the captured sound buffer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundBuffer& getBuffer() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Reset internal state in preparation for a new capture
    ///
    /// Discards any samples held from a previous recording so
    /// that the buffer only contains samples from the new run.
    ///
    /// \param captureDevice Capture device the recording is starting on
    ///
    /// \return Always `true`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStart(CaptureDevice& captureDevice) override;

    ////////////////////////////////////////////////////////////
    /// \brief Process a new chunk of recorded samples
    ///
    /// Appends the chunk to the internal sample buffer.
    ///
    /// \param samples     Pointer to the new chunk of recorded samples (interleaved 16-bit signed PCM)
    /// \param sampleCount Number of samples pointed to by `samples`
    ///
    /// \return Always `true` (capture continues)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onProcessSamples(const base::I16* samples, base::SizeT sampleCount) override;

    ////////////////////////////////////////////////////////////
    /// \brief Finalize the captured sound buffer
    ///
    /// Builds the final `sf::SoundBuffer` from the accumulated
    /// samples once the capture has stopped.
    ///
    /// \param captureDevice Capture device the recording is stopping on
    ///
    /// \return `true` on success, `false` if the buffer could not be built
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStop(CaptureDevice& captureDevice) override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 384> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundBufferRecorder
/// \ingroup audio
///
/// `sf::SoundBufferRecorder` is the simplest concrete
/// `sf::SoundRecorder`: every recorded chunk is appended into
/// an internal `sf::SoundBuffer`, which can then be retrieved
/// (and played, saved to a file, etc.) once the recording has
/// stopped.
///
/// It has the same interface as its base class
/// (`start(captureDevice)` / `stop()`) and only adds the
/// `getBuffer()` accessor.
///
/// Usage example:
/// \code
/// // Open the default capture device
/// auto audioContext  = sf::AudioContext::create().value();
/// auto deviceHandle  = sf::AudioContext::getDefaultCaptureDeviceHandle().value();
/// sf::CaptureDevice  captureDevice{deviceHandle};
///
/// // Record some audio data
/// sf::SoundBufferRecorder recorder;
/// if (!recorder.start(captureDevice))
/// {
///     // Handle error...
/// }
///
/// // ... record for a while ...
///
/// (void)recorder.stop();
///
/// // Get the buffer containing the captured audio data
/// const sf::SoundBuffer& buffer = recorder.getBuffer();
///
/// // Save it to a file (for example)
/// if (!buffer.saveToFile("my_record.ogg"))
/// {
///     // Handle error...
/// }
/// \endcode
///
/// \see `sf::SoundRecorder`, `sf::CaptureDevice`
///
////////////////////////////////////////////////////////////
