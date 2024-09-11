#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/SizeT.hpp"

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CaptureDevice;
class Time;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for capturing sound data
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundRecorder
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~SoundRecorder();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundRecorder(const SoundRecorder&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundRecorder& operator=(const SoundRecorder&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundRecorder(SoundRecorder&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    SoundRecorder& operator=(SoundRecorder&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Start the capture
    ///
    /// The \a sampleRate parameter defines the number of audio samples
    /// captured per second. The higher, the better the quality
    /// (for example, 44100 samples/sec is CD quality).
    /// This function uses its own thread so that it doesn't block
    /// the rest of the program while the capture runs.
    /// Please note that only one capture can happen at the same time.
    /// You can select which capture device will be used by passing
    /// the name to the setCurrentDevice() method. If none was selected
    /// before, the default capture device will be used. You can get a
    /// list of the names of all available capture devices by calling
    /// getAvailableDevices().
    ///
    /// \param sampleRate Desired capture rate, in number of samples per second
    ///
    /// \return True, if start of capture was successful
    ///
    /// \see stop, getAvailableDevices
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool start(CaptureDevice& captureDevice, unsigned int sampleRate = 44100);

    ////////////////////////////////////////////////////////////
    /// \brief Stop the capture
    ///
    /// \see start
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool stop();


protected:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor is only meant to be called by derived classes.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SoundRecorder();

    ////////////////////////////////////////////////////////////
    /// \brief Start capturing audio data
    ///
    /// This virtual function may be overridden by a derived class
    /// if something has to be done every time a new capture
    /// starts. If not, this function can be ignored; the default
    /// implementation does nothing.
    ///
    /// \return True to start the capture, or false to abort it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool onStart(CaptureDevice& captureDevice);

    ////////////////////////////////////////////////////////////
    /// \brief Process a new chunk of recorded samples
    ///
    /// This virtual function is called every time a new chunk of
    /// recorded data is available. The derived class can then do
    /// whatever it wants with it (storing it, playing it, sending
    /// it over the network, etc.).
    ///
    /// \param samples     Pointer to the new chunk of recorded samples
    /// \param sampleCount Number of samples pointed by \a samples
    ///
    /// \return True to continue the capture, or false to stop it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool onProcessSamples(const std::int16_t* samples, base::SizeT sampleCount) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Stop capturing audio data
    ///
    /// This virtual function may be overridden by a derived class
    /// if something has to be done every time the capture
    /// ends. If not, this function can be ignored; the default
    /// implementation does nothing.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool onStop(CaptureDevice& captureDevice);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    CaptureDevice* m_lastCaptureDevice{};

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(CaptureDevice);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundRecorder
/// \ingroup audio
///
/// sf::SoundBuffer provides a simple interface to access
/// the audio recording capabilities of the computer
/// (the microphone). As an abstract base class, it only cares
/// about capturing sound samples, the task of making something
/// useful with them is left to the derived class. Note that
/// SFML provides a built-in specialization for saving the
/// captured data to a sound buffer (see sf::SoundBufferRecorder).
///
/// A derived class has only one virtual function to override:
/// \li onProcessSamples provides the new chunks of audio samples while the capture happens
///
/// Moreover, two additional virtual functions can be overridden
/// as well if necessary:
/// \li onStart is called before the capture happens, to perform custom initializations
/// \li onStop is called after the capture ends, to perform custom cleanup
///
/// A derived class can also control the frequency of the onProcessSamples
/// calls, with the setProcessingInterval protected function. The default
/// interval is chosen so that recording thread doesn't consume too much
/// CPU, but it can be changed to a smaller value if you need to process
/// the recorded data in real time, for example.
///
/// The audio capture feature may not be supported or activated
/// on every platform, thus it is recommended to check its
/// availability with the isAvailable() function. If it returns
/// false, then any attempt to use an audio recorder will fail.
///
/// If you have multiple sound input devices connected to your
/// computer (for example: microphone, external soundcard, webcam mic, ...)
/// you can get a list of all available devices through the
/// getAvailableDevices() function. You can then select a device
/// by calling setCurrentDevice() with the appropriate device. Otherwise
/// the default capturing device will be used.
///
/// By default the recording is in 16-bit mono. Using the
/// setChannelCount method you can change the number of channels
/// used by the audio capture device to record. Note that you
/// have to decide whether you want to record in mono or stereo
/// before starting the recording.
///
/// It is important to note that the audio capture happens in a
/// separate thread, so that it doesn't block the rest of the
/// program. In particular, the onProcessSamples virtual function
/// (but not onStart and not onStop) will be called
/// from this separate thread. It is important to keep this in
/// mind, because you may have to take care of synchronization
/// issues if you share data between threads.
/// Another thing to bear in mind is that you must call stop()
/// in the destructor of your derived class, so that the recording
/// thread finishes before your object is destroyed.
///
/// Usage example:
/// \code
/// class CustomRecorder : public sf::SoundRecorder
/// {
/// public:
///     ~CustomRecorder()
///     {
///         // Make sure to stop the recording thread
///         stop();
///     }
///
/// private:
///     bool onStart() override // optional
///     {
///         // Initialize whatever has to be done before the capture starts
///         ...
///
///         // Return true to start playing
///         return true;
///     }
///
///     [[nodiscard]] bool onProcessSamples(const std::int16_t* samples, base::SizeT sampleCount) override
///     {
///         // Do something with the new chunk of samples (store them, send them, ...)
///         ...
///
///         // Return true to continue playing
///         return true;
///     }
///
///     void onStop() override // optional
///     {
///         // Clean up whatever has to be done after the capture ends
///         ...
///     }
/// };
///
/// // Usage
/// if (CustomRecorder::isAvailable())
/// {
///     CustomRecorder recorder;
///
///     if (!recorder.start())
///         return -1;
///
///     ...
///     recorder.stop();
/// }
/// \endcode
///
/// \see sf::SoundBufferRecorder
///
////////////////////////////////////////////////////////////
