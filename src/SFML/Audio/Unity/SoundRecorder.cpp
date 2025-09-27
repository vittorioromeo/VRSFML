// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/Audio/CaptureDevice.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
SoundRecorder::SoundRecorder() = default;


////////////////////////////////////////////////////////////
SoundRecorder::~SoundRecorder()
{
    SFML_BASE_ASSERT(m_lastCaptureDevice == nullptr &&
                     "The sound recorder must be stopped before destroying the capture device");
}


////////////////////////////////////////////////////////////
bool SoundRecorder::start(CaptureDevice& captureDevice, unsigned int sampleRate)
{
    // Store the sample rate and re-initialize if necessary
    if (!captureDevice.setSampleRate(sampleRate))
        return false;

    // Ensure we have a capture device
    if (!captureDevice.isDeviceInitialized())
    {
        priv::err() << "Trying to start sound recorder, but no device available";
        return false;
    }

    // Check that another capture is not already running
    if (captureDevice.isDeviceStarted())
    {
        priv::err() << "Trying to start sound recorder, but another capture is already running";
        return false;
    }

    // Notify derived class
    if (!onStart(captureDevice))
        return false;

    // Start the capture
    if (!captureDevice.startDevice())
    {
        priv::err() << "Failed to start sound recorder";
        return false;
    }

    m_lastCaptureDevice = &captureDevice;
    SFML_UPDATE_LIFETIME_DEPENDANT(CaptureDevice, SoundRecorder, this, m_lastCaptureDevice);

    captureDevice.setProcessSamplesFunc(this, [](void* userData, const base::I16* samples, base::SizeT sampleCount) {
        return static_cast<SoundRecorder*>(userData)->onProcessSamples(samples, sampleCount);
    });

    return true;
}


////////////////////////////////////////////////////////////
bool SoundRecorder::stop()
{
    if (m_lastCaptureDevice == nullptr) // Already stopped
        return true;

    auto* const savedCaptureDevice = m_lastCaptureDevice;
    m_lastCaptureDevice            = nullptr;

    savedCaptureDevice->setProcessSamplesFunc(nullptr, nullptr);

    if (!savedCaptureDevice->isDeviceInitialized() || !savedCaptureDevice->isDeviceStarted())
        return false;

    if (!savedCaptureDevice->stopDevice())
    {
        priv::err() << "Failed to stop sound recorder";
        return false;
    }

    // Notify derived class
    return onStop(*savedCaptureDevice);
}


////////////////////////////////////////////////////////////
bool SoundRecorder::onStart(CaptureDevice&)
{
    return true;
}


////////////////////////////////////////////////////////////
bool SoundRecorder::onStop(CaptureDevice&)
{
    return true;
}

} // namespace sf
