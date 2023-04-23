////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/ALCheck.hpp>
#include <SFML/Audio/AudioDevice.hpp>
#include <SFML/Audio/SoundRecorder.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <mutex>
#include <ostream>
#include <thread>
#include <vector>

#include <cassert>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable : 4355) // 'this' used in base member initializer list
#endif

#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace
{
ALCdevice* captureDevice = nullptr;
}

namespace sf
{
////////////////////////////////////////////////////////////
class SoundRecorder::Impl
{
public:
    Impl(SoundRecorder* parent);
    ~Impl();

    [[nodiscard]] bool start(unsigned int sampleRate = 44100);

    void stop();

    unsigned int getSampleRate() const;

    static std::vector<std::string> getAvailableDevices();

    static std::string getDefaultDevice();

    [[nodiscard]] bool setDevice(const std::string& name);

    const std::string& getDevice() const;

    void setChannelCount(unsigned int channelCount);

    unsigned int getChannelCount() const;

    static bool isAvailable();

    void setProcessingInterval(Time interval);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Function called as the entry point of the thread
    ///
    /// This function starts the recording loop, and returns
    /// only when the capture is stopped.
    ///
    ////////////////////////////////////////////////////////////
    void record();

    ////////////////////////////////////////////////////////////
    /// \brief Get the new available audio samples and process them
    ///
    /// This function is called continuously during the
    /// capture loop. It retrieves the captured samples and
    /// forwards them to the derived class.
    ///
    ////////////////////////////////////////////////////////////
    void processCapturedSamples();

    ////////////////////////////////////////////////////////////
    /// \brief Clean up the recorder's internal resources
    ///
    /// This function is called when the capture stops.
    ///
    ////////////////////////////////////////////////////////////
    void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Launch a new capture thread running 'record'
    ///
    /// This function is called when the capture is started or
    /// when the device is changed.
    ///
    ////////////////////////////////////////////////////////////
    void launchCapturingThread();

    ////////////////////////////////////////////////////////////
    /// \brief Stop capturing and wait for 'm_thread' to join
    ///
    /// This function is called when the capture is stopped or
    /// when the device is changed.
    ///
    ////////////////////////////////////////////////////////////
    void awaitCapturingThread();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SoundRecorder*            m_parent;
    std::thread               m_thread;                   //!< Thread running the background recording task
    std::vector<std::int16_t> m_samples;                  //!< Buffer to store captured samples
    unsigned int              m_sampleRate{};             //!< Sample rate
    Time         m_processingInterval{milliseconds(100)}; //!< Time period between calls to onProcessSamples
    bool         m_isCapturing{};                         //!< Capturing state
    std::string  m_deviceName{getDefaultDevice()};        //!< Name of the audio capture device
    unsigned int m_channelCount{1};                       //!< Number of recording channels
};

////////////////////////////////////////////////////////////
SoundRecorder::Impl::Impl(SoundRecorder* parent) : m_parent(parent)
{
    assert(parent != nullptr);
}


////////////////////////////////////////////////////////////
SoundRecorder::Impl::~Impl()
{
    // This assertion is triggered if the recording is still running while
    // the object is destroyed. It ensures that stop() is called in the
    // destructor of the derived class, which makes sure that the recording
    // thread finishes before the derived object is destroyed. Otherwise a
    // "pure virtual method called" exception is triggered.
    assert(!m_isCapturing &&
           "You must call stop() in the destructor of your derived class, so that the recording thread finishes before "
           "your object is destroyed.");
}


////////////////////////////////////////////////////////////
bool SoundRecorder::Impl::start(unsigned int sampleRate)
{
    // Check if the device can do audio capture
    if (!isAvailable())
    {
        err() << "Failed to start capture: your system cannot capture audio data (call "
                 "SoundRecorder::Impl::isAvailable to "
                 "check it)"
              << std::endl;
        return false;
    }

    // Check that another capture is not already running
    if (captureDevice)
    {
        err() << "Trying to start audio capture, but another capture is already running" << std::endl;
        return false;
    }

    // Determine the recording format
    ALCenum format = (m_channelCount == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    // Open the capture device for capturing 16 bits samples
    captureDevice = alcCaptureOpenDevice(m_deviceName.c_str(), sampleRate, format, static_cast<ALCsizei>(sampleRate));
    if (!captureDevice)
    {
        err() << "Failed to open the audio capture device with the name: " << m_deviceName << std::endl;
        return false;
    }

    // Clear the array of samples
    m_samples.clear();

    // Store the sample rate
    m_sampleRate = sampleRate;

    // Notify derived class
    if (m_parent->onStart())
    {
        // Start the capture
        alcCaptureStart(captureDevice);

        // Start the capture in a new thread, to avoid blocking the main thread
        launchCapturingThread();

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::stop()
{
    // Stop the capturing thread if there is one
    if (m_isCapturing)
    {
        awaitCapturingThread();

        // Notify derived class
        m_parent->onStop();
    }
}


////////////////////////////////////////////////////////////
unsigned int SoundRecorder::Impl::getSampleRate() const
{
    return m_sampleRate;
}


////////////////////////////////////////////////////////////
std::vector<std::string> SoundRecorder::Impl::getAvailableDevices()
{
    std::vector<std::string> deviceNameList;

    const ALchar* deviceList = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (deviceList)
    {
        while (*deviceList)
        {
            deviceNameList.emplace_back(deviceList);
            deviceList += std::strlen(deviceList) + 1;
        }
    }

    return deviceNameList;
}


////////////////////////////////////////////////////////////
std::string SoundRecorder::Impl::getDefaultDevice()
{
    return alcGetString(nullptr, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
}


////////////////////////////////////////////////////////////
bool SoundRecorder::Impl::setDevice(const std::string& name)
{
    // Store the device name
    if (name.empty())
        m_deviceName = getDefaultDevice();
    else
        m_deviceName = name;

    if (m_isCapturing)
    {
        // Stop the capturing thread
        awaitCapturingThread();

        // Determine the recording format
        ALCenum format = (m_channelCount == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

        // Open the requested capture device for capturing 16 bits samples
        captureDevice = alcCaptureOpenDevice(m_deviceName.c_str(), m_sampleRate, format, static_cast<ALCsizei>(m_sampleRate));
        if (!captureDevice)
        {
            // Notify derived class
            m_parent->onStop();

            err() << "Failed to open the audio capture device with the name: " << m_deviceName << std::endl;
            return false;
        }

        // Start the capture
        alcCaptureStart(captureDevice);

        // Start the capture in a new thread, to avoid blocking the main thread
        launchCapturingThread();
    }

    return true;
}


////////////////////////////////////////////////////////////
const std::string& SoundRecorder::Impl::getDevice() const
{
    return m_deviceName;
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::setChannelCount(unsigned int channelCount)
{
    if (m_isCapturing)
    {
        err() << "It's not possible to change the channels while recording." << std::endl;
        return;
    }

    if (channelCount < 1 || channelCount > 2)
    {
        err() << "Unsupported channel count: " << channelCount
              << " Currently only mono (1) and stereo (2) recording is supported." << std::endl;
        return;
    }

    m_channelCount = channelCount;
}


////////////////////////////////////////////////////////////
unsigned int SoundRecorder::Impl::getChannelCount() const
{
    return m_channelCount;
}


////////////////////////////////////////////////////////////
bool SoundRecorder::Impl::isAvailable()
{
    return (priv::AudioDevice::isExtensionSupported("ALC_EXT_CAPTURE") != AL_FALSE) ||
           (priv::AudioDevice::isExtensionSupported("ALC_EXT_capture") != AL_FALSE); // "bug" in Mac OS X 10.5 and 10.6
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::setProcessingInterval(Time interval)
{
    m_processingInterval = interval;
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::record()
{
    while (m_isCapturing)
    {
        // Process available samples
        processCapturedSamples();

        // Don't bother the CPU while waiting for more captured data
        sleep(m_processingInterval);
    }

    // Capture is finished: clean up everything
    cleanup();
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::processCapturedSamples()
{
    // Get the number of samples available
    ALCint samplesAvailable;
    alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

    if (samplesAvailable > 0)
    {
        // Get the recorded samples
        m_samples.resize(static_cast<std::size_t>(samplesAvailable) * getChannelCount());
        alcCaptureSamples(captureDevice, m_samples.data(), samplesAvailable);

        // Forward them to the derived class
        if (!m_parent->onProcessSamples(m_samples.data(), m_samples.size()))
        {
            // The user wants to stop the capture
            m_isCapturing = false;
        }
    }
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::cleanup()
{
    // Stop the capture
    alcCaptureStop(captureDevice);

    // Get the samples left in the buffer
    processCapturedSamples();

    // Close the device
    alcCaptureCloseDevice(captureDevice);
    captureDevice = nullptr;
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::launchCapturingThread()
{
    m_isCapturing = true;

    assert(!m_thread.joinable());
    m_thread = std::thread(&SoundRecorder::Impl::record, this);
}


////////////////////////////////////////////////////////////
void SoundRecorder::Impl::awaitCapturingThread()
{
    m_isCapturing = false;

    if (m_thread.joinable())
        m_thread.join();
}


////////////////////////////////////////////////////////////
SoundRecorder::~SoundRecorder() = default;


////////////////////////////////////////////////////////////
bool SoundRecorder::start(unsigned int sampleRate)
{
    return m_impl->start(sampleRate);
}


////////////////////////////////////////////////////////////
void SoundRecorder::stop()
{
    return m_impl->stop();
}


////////////////////////////////////////////////////////////
unsigned int SoundRecorder::getSampleRate() const
{
    return m_impl->getSampleRate();
}


////////////////////////////////////////////////////////////
std::vector<std::string> SoundRecorder::getAvailableDevices()
{
    return Impl::getAvailableDevices();
}


////////////////////////////////////////////////////////////
std::string SoundRecorder::getDefaultDevice()
{
    return Impl::getDefaultDevice();
}


////////////////////////////////////////////////////////////
bool SoundRecorder::setDevice(const std::string& name)
{
    return m_impl->setDevice(name);
}


////////////////////////////////////////////////////////////
const std::string& SoundRecorder::getDevice() const
{
    return m_impl->getDevice();
}


////////////////////////////////////////////////////////////
void SoundRecorder::setChannelCount(unsigned int channelCount)
{
    m_impl->setChannelCount(channelCount);
}


////////////////////////////////////////////////////////////
unsigned int SoundRecorder::getChannelCount() const
{
    return m_impl->getChannelCount();
}


////////////////////////////////////////////////////////////
bool SoundRecorder::isAvailable()
{
    return Impl::isAvailable();
}


////////////////////////////////////////////////////////////
SoundRecorder::SoundRecorder() : m_impl(sf::priv::makeUnique<Impl>(this))
{
}


////////////////////////////////////////////////////////////
void SoundRecorder::setProcessingInterval(Time interval)
{
    m_impl->setProcessingInterval(interval);
}


////////////////////////////////////////////////////////////
bool SoundRecorder::onStart()
{
    // Nothing to do.
    return true;
}


////////////////////////////////////////////////////////////
void SoundRecorder::onStop()
{
    // Nothing to do.
}

} // namespace sf
