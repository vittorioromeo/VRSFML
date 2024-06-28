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
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundBufferRecorder.hpp>

#include <SFML/System/Err.hpp>

#include <optional>
#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundBufferRecorder::Impl
{
    std::vector<std::int16_t>  samples; //!< Temporary sample buffer to hold the recorded data
    std::optional<SoundBuffer> buffer;  //!< Sound buffer that will contain the recorded data
};


////////////////////////////////////////////////////////////
SoundBufferRecorder::SoundBufferRecorder() = default;


////////////////////////////////////////////////////////////
SoundBufferRecorder::~SoundBufferRecorder()
{
    // Make sure to stop the recording thread
    stop();
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onStart()
{
    m_impl->samples.clear();
    m_impl->buffer.reset();

    return true;
}


////////////////////////////////////////////////////////////
bool SoundBufferRecorder::onProcessSamples(const std::int16_t* samples, std::size_t sampleCount)
{
    for (const std::int16_t* p = samples; p != p + sampleCount; ++p)
        m_impl->samples.push_back(*p);

    return true;
}


////////////////////////////////////////////////////////////
void SoundBufferRecorder::onStop()
{
    if (m_impl->samples.empty())
        return;

    m_impl->buffer = sf::SoundBuffer::loadFromSamples(m_impl->samples.data(),
                                                      m_impl->samples.size(),
                                                      getChannelCount(),
                                                      getSampleRate(),
                                                      getChannelMap());

    if (!m_impl->buffer)
        priv::err() << "Failed to stop capturing audio data" << priv::errEndl;
}


////////////////////////////////////////////////////////////
const SoundBuffer& SoundBufferRecorder::getBuffer() const
{
    assert(m_impl->buffer && "SoundBufferRecorder::getBuffer() Cannot return reference to null buffer");
    return *m_impl->buffer;
}

} // namespace sf
