#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/Base/InPlacePImpl.hpp"

#include <cstddef>
#include <cstdint>


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
    /// The sound buffer is valid only after the capture has ended.
    /// This function provides a read-only access to the internal
    /// sound buffer, but it can be copied if you need to
    /// make any modification to it.
    ///
    /// \return Read-only access to the sound buffer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundBuffer& getBuffer() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Start capturing audio data
    ///
    /// \return True to start the capture, or false to abort it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStart(CaptureDevice& captureDevice) override;

    ////////////////////////////////////////////////////////////
    /// \brief Process a new chunk of recorded samples
    ///
    /// \param samples     Pointer to the new chunk of recorded samples
    /// \param sampleCount Number of samples pointed by \a samples
    ///
    /// \return True to continue the capture, or false to stop it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onProcessSamples(const std::int16_t* samples, std::size_t sampleCount) override;

    ////////////////////////////////////////////////////////////
    /// \brief Stop capturing audio data
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onStop(CaptureDevice& captureDevice) override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 256> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundBufferRecorder
/// \ingroup audio
///
/// sf::SoundBufferRecorder allows to access a recorded sound
/// through a sf::SoundBuffer, so that it can be played, saved
/// to a file, etc.
///
/// It has the same simple interface as its base class (start(), stop())
/// and adds a function to retrieve the recorded sound buffer
/// (getBuffer()).
///
/// As usual, don't forget to call the isAvailable() function
/// before using this class (see sf::SoundRecorder for more details
/// about this).
///
/// Usage example:
/// \code
/// if (sf::SoundBufferRecorder::isAvailable())
/// {
///     // Record some audio data
///     sf::SoundBufferRecorder recorder;
///     if (!recorder.start())
///     {
///         // Handle error...
///     }
///     ...
///     recorder.stop();
///
///     // Get the buffer containing the captured audio data
///     const sf::SoundBuffer& buffer = recorder.getBuffer();
///
///     // Save it to a file (for example...)
///     if (!buffer.saveToFile("my_record.ogg"))
///     {
///         // Handle error...
///     }
/// }
/// \endcode
///
/// \see sf::SoundRecorder
///
////////////////////////////////////////////////////////////
