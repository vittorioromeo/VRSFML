////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
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
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/SoundRecorder.hpp>

#include <SFML/System/Assert.hpp>
#include <SFML/System/Err.hpp>

#include <miniaudio.h>

#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
SoundRecorder::SoundRecorder() = default;


////////////////////////////////////////////////////////////
SoundRecorder::~SoundRecorder()
{
    SFML_ASSERT(m_lastCaptureDevice == nullptr &&
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
        priv::err() << "Trying to start sound recorder, but no device available" << priv::errEndl;
        return false;
    }

    // Check that another capture is not already running
    if (captureDevice.isDeviceStarted())
    {
        priv::err() << "Trying to start sound recorder, but another capture is already running" << priv::errEndl;
        return false;
    }

    // Notify derived class
    if (!onStart(captureDevice))
        return false;

    // Start the capture
    if (!captureDevice.startDevice())
    {
        priv::err() << "Failed to start sound recorder" << priv::errEndl;
        return false;
    }

    m_lastCaptureDevice = &captureDevice;
    SFML_UPDATE_LIFETIME_DEPENDANT(CaptureDevice, SoundRecorder, this, m_lastCaptureDevice);

    captureDevice.setProcessSamplesFunc(this,
                                        [](void* userData, const std::int16_t* samples, std::size_t sampleCount) {
                                            return static_cast<SoundRecorder*>(userData)->onProcessSamples(samples,
                                                                                                           sampleCount);
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
        priv::err() << "Failed to stop sound recorder" << priv::errEndl;
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
