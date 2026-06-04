// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/CaptureDevice.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"
#include "Zancle/Audio/SoundBufferRecorder.hpp"
#include "Zancle/Audio/SoundRecorder.hpp"
#include "Zancle/System/Err.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Vector.hpp"


namespace za
{
////////////////////////////////////////////////////////////
struct SoundBufferRecorder::Impl
{
    zb::Vector<zb::I16>       samples; //!< Temporary sample buffer to hold the recorded data
    zb::Optional<SoundBuffer> buffer;  //!< Sound buffer that will contain the recorded data
};


////////////////////////////////////////////////////////////
SoundBufferRecorder::SoundBufferRecorder() = default;


////////////////////////////////////////////////////////////
SoundBufferRecorder::~SoundBufferRecorder()
{
    if (!stop())
        priv::errMsg("Failed to stop sound buffer recorder on destruction");
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onStart(CaptureDevice&)
{
    m_impl->samples.clear();
    m_impl->buffer.reset();

    return true;
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onProcessSamples(const zb::I16* samples, zb::SizeT sampleCount)
{
    const zb::SizeT oldSize = m_impl->samples.size();
    m_impl->samples.resize(oldSize + sampleCount);

    ZB_MEMCPY(m_impl->samples.data() + oldSize, samples, sampleCount * sizeof(zb::I16));

    return true;
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onStop(CaptureDevice& captureDevice)
{
    if (m_impl->samples.empty())
        return true;

    m_impl->buffer = za::SoundBuffer::loadFromSamples(m_impl->samples.data(),
                                                      m_impl->samples.size(),
                                                      captureDevice.getChannelMap(),
                                                      captureDevice.getSampleRate());

    if (!m_impl->buffer.hasValue())
    {
        priv::errMsg("Failed to stop capturing audio data");
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
const SoundBuffer& SoundBufferRecorder::getBuffer() const
{
    ZB_ASSERT(m_impl->buffer.hasValue() && "SoundBufferRecorder::getBuffer() Cannot return reference to null buffer");
    return *m_impl->buffer;
}

} // namespace za
