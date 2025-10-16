// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundBufferRecorder.hpp"

#include "SFML/Audio/CaptureDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundRecorder.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundBufferRecorder::Impl
{
    base::Vector<base::I16>     samples; //!< Temporary sample buffer to hold the recorded data
    base::Optional<SoundBuffer> buffer;  //!< Sound buffer that will contain the recorded data
};


////////////////////////////////////////////////////////////
SoundBufferRecorder::SoundBufferRecorder() = default;


////////////////////////////////////////////////////////////
SoundBufferRecorder::~SoundBufferRecorder()
{
    if (!stop())
        priv::err() << "Failed to stop sound buffer recorder on destruction";
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onStart(CaptureDevice&)
{
    m_impl->samples.clear();
    m_impl->buffer.reset();

    return true;
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onProcessSamples(const base::I16* samples, base::SizeT sampleCount)
{
    const base::SizeT oldSize = m_impl->samples.size();
    m_impl->samples.resize(oldSize + sampleCount);

    SFML_BASE_MEMCPY(m_impl->samples.data() + oldSize, samples, sampleCount * sizeof(base::I16));

    return true;
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onStop(CaptureDevice& captureDevice)
{
    if (m_impl->samples.empty())
        return true;

    m_impl->buffer = sf::SoundBuffer::loadFromSamples(m_impl->samples.data(),
                                                      m_impl->samples.size(),
                                                      captureDevice.getChannelMap(),
                                                      captureDevice.getSampleRate());

    if (!m_impl->buffer.hasValue())
    {
        priv::err() << "Failed to stop capturing audio data";
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
const SoundBuffer& SoundBufferRecorder::getBuffer() const
{
    SFML_BASE_ASSERT(m_impl->buffer.hasValue() &&
                     "SoundBufferRecorder::getBuffer() Cannot return reference to null buffer");
    return *m_impl->buffer;
}

} // namespace sf
