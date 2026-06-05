// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/SoundBufferRecorder.hpp"

#include "Zancle/Audio/CaptureDevice.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"
#include "Zancle/Audio/SoundRecorder.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/Memcpy.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Container/Vector.hpp"


namespace za
{
////////////////////////////////////////////////////////////
struct SoundBufferRecorder::Impl
{
    za::Vector<za::I16>       samples; //!< Temporary sample buffer to hold the recorded data
    za::Optional<SoundBuffer> buffer;  //!< Sound buffer that will contain the recorded data
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
bool SoundBufferRecorder::onProcessSamples(const za::I16* samples, za::SizeT sampleCount)
{
    const za::SizeT oldSize = m_impl->samples.size();
    m_impl->samples.resize(oldSize + sampleCount);

    ZA_MEMCPY(m_impl->samples.data() + oldSize, samples, sampleCount * sizeof(za::I16));

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
    ZA_ASSERT(m_impl->buffer.hasValue() && "SoundBufferRecorder::getBuffer() Cannot return reference to null buffer");
    return *m_impl->buffer;
}

} // namespace za
