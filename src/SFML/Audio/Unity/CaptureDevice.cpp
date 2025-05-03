#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioContextUtils.hpp"
#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Vector.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct CaptureDevice::Impl
{
    static void maDeviceDataCallback(ma_device* device, void*, const void* input, ma_uint32 frameCount)
    {
        auto& impl = *static_cast<Impl*>(device->pUserData);

        // Copy the new samples into our temporary buffer
        impl.samples.resize(frameCount * impl.channelCount);
        SFML_BASE_MEMCPY(impl.samples.data(), input, frameCount * impl.channelCount * sizeof(base::I16));

        // Notify the derived class of the availability of new samples
        SFML_BASE_ASSERT(impl.processSamplesFunc != nullptr &&
                         "processSamplesFunc callback not registered in capture device");
        SFML_BASE_ASSERT(impl.soundRecorder != nullptr && "processSamplesFunc callback user data is null");
        if (impl.processSamplesFunc(impl.soundRecorder, impl.samples.data(), impl.samples.size()))
            return;

        // If the derived class wants to stop, stop the capture
        if (const auto result = ma_device_stop(device); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("stop audio capture device", result);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] explicit Impl(AudioContext& theAudioContext,
                                // NOLINTNEXTLINE(modernize-pass-by-value)
                                const CaptureDeviceHandle& theDeviceHandle) :
    audioContext(&theAudioContext),
    captureDeviceHandle(theDeviceHandle)
    {
    }

    ~Impl()
    {
        deinitialize();
    }

    void deinitialize()
    {
        ma_device_uninit(&maDevice);
    }

    [[nodiscard]] bool initialize()
    {
        ma_device_config captureDeviceConfig = ma_device_config_init(ma_device_type_capture);
        captureDeviceConfig.dataCallback     = maDeviceDataCallback;
        captureDeviceConfig.pUserData        = this;
        captureDeviceConfig.capture
            .pDeviceID = &static_cast<const ma_device_info*>(captureDeviceHandle.getMADeviceInfo())->id;
        captureDeviceConfig.capture.channels = channelCount;
        captureDeviceConfig.capture.format   = ma_format_s16;
        captureDeviceConfig.sampleRate       = sampleRate;

        if (const auto result = ma_device_init(static_cast<ma_context*>(audioContext->getMAContext()),
                                               &captureDeviceConfig,
                                               &maDevice);
            result != MA_SUCCESS)
            return priv::MiniaudioUtils::fail("initialize the audio device", result);

        return true;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    AudioContext*           audioContext;                   //!< Audio context
    CaptureDeviceHandle     captureDeviceHandle;            //!< Capture device handle
    ma_uint32               channelCount{1u};               //!< Number of recording channels
    ma_uint32               sampleRate{44'100u};            //!< Sample rate
    base::Vector<base::I16> samples;                        //!< Buffer to store captured samples
    ChannelMap              channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel

    SoundRecorder*     soundRecorder{nullptr}; //!< Used in the miniaudio device callback
    ProcessSamplesFunc processSamplesFunc{};   //!< Used in the miniaudio device callback

    ma_device maDevice; //!< miniaudio capture device (one per hardware device)
};


////////////////////////////////////////////////////////////
base::Optional<CaptureDevice> CaptureDevice::createDefault(AudioContext& audioContext)
{
    base::Optional defaultCaptureDeviceHandle = AudioContextUtils::getDefaultCaptureDeviceHandle(audioContext);

    if (!defaultCaptureDeviceHandle.hasValue())
        return base::nullOpt;

    return base::makeOptional<CaptureDevice>(audioContext, *defaultCaptureDeviceHandle);
}


////////////////////////////////////////////////////////////
CaptureDevice::CaptureDevice(AudioContext& audioContext, const CaptureDeviceHandle& playbackDeviceHandle) :
m_impl(base::makeUnique<Impl>(audioContext, playbackDeviceHandle))
{
    if (!m_impl->initialize())
        priv::err() << "Failed to initialize the capture device";

    SFML_UPDATE_LIFETIME_DEPENDANT(AudioContext, CaptureDevice, this, m_impl->audioContext);
}


////////////////////////////////////////////////////////////
CaptureDevice::~CaptureDevice()
{
    if (m_impl == nullptr) // Could be moved-from
        return;

    SFML_BASE_ASSERT(!ma_device_is_started(&m_impl->maDevice) &&
                     "The miniaudio capture device must be stopped before destroying the capture device");
}


////////////////////////////////////////////////////////////
CaptureDevice::CaptureDevice(CaptureDevice&&) noexcept = default;


////////////////////////////////////////////////////////////
CaptureDevice& CaptureDevice::operator=(CaptureDevice&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] const CaptureDeviceHandle& CaptureDevice::getDeviceHandle() const
{
    return m_impl->captureDeviceHandle;
}


////////////////////////////////////////////////////////////
bool CaptureDevice::setSampleRate(unsigned int sampleRate)
{
    // Store the sample rate and re-initialize if necessary
    if (m_impl->sampleRate != sampleRate)
    {
        m_impl->sampleRate = sampleRate;

        m_impl->deinitialize();
        if (!m_impl->initialize())
        {
            priv::err() << "Failed to set audio capture device sample rate to " << sampleRate;
            return false;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
unsigned int CaptureDevice::getSampleRate() const
{
    return m_impl->sampleRate;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::isDeviceInitialized() const
{
    return ma_device_get_state(&m_impl->maDevice) != ma_device_state_uninitialized;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::isDeviceStarted() const
{
    return ma_device_is_started(&m_impl->maDevice);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::startDevice() const
{
    SFML_BASE_ASSERT(isDeviceInitialized() && "Attempted to start an uninitialized audio capture device");
    SFML_BASE_ASSERT(!isDeviceStarted() && "Attempted to start an already started audio capture device");

    if (const auto result = ma_device_start(&m_impl->maDevice); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("start audio capture device", result);

    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::stopDevice() const
{
    SFML_BASE_ASSERT(isDeviceInitialized() && "Attempted to stop an uninitialized audio capture device");
    SFML_BASE_ASSERT(isDeviceStarted() && "Attempted to stop an already stopped audio capture device");

    if (const auto result = ma_device_stop(&m_impl->maDevice); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("stop audio capture device", result);

    return true;
}


////////////////////////////////////////////////////////////
bool CaptureDevice::setChannelCount(unsigned int channelCount)
{
    // We only bother supporting mono/stereo recording for now
    if (channelCount < 1 || channelCount > 2)
    {
        priv::err() << "Unsupported channel count: " << channelCount
                    << " Currently only mono (1) and stereo (2) recording is supported.";

        return false;
    }

    // Store the channel count and re-initialize if necessary
    if (m_impl->channelCount == channelCount)
        return true;

    m_impl->channelCount = channelCount;

    m_impl->deinitialize();
    if (!m_impl->initialize())
    {
        priv::err() << "Failed to set audio capture device channel count to " << channelCount;
        return false;
    }

    // We only bother supporting mono/stereo recording for now
    if (channelCount == 1)
    {
        m_impl->channelMap = {SoundChannel::Mono};
    }
    else if (channelCount == 2)
    {
        m_impl->channelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight};
    }

    return true;
}


////////////////////////////////////////////////////////////
unsigned int CaptureDevice::getChannelCount() const
{
    return m_impl->channelCount;
}


////////////////////////////////////////////////////////////
const ChannelMap& CaptureDevice::getChannelMap() const
{
    return m_impl->channelMap;
}


////////////////////////////////////////////////////////////
void CaptureDevice::setProcessSamplesFunc(SoundRecorder* soundRecorder, ProcessSamplesFunc processSamplesFunc)
{
    m_impl->soundRecorder      = soundRecorder;
    m_impl->processSamplesFunc = processSamplesFunc;
}

} // namespace sf
