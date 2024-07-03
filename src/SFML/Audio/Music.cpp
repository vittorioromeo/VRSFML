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
#include <SFML/Audio/InputSoundFile.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/MusicStream.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/Time.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
Music::~Music() = default;


////////////////////////////////////////////////////////////
Music::Music(Music&&) noexcept = default;


////////////////////////////////////////////////////////////
Music& Music::operator=(Music&&) noexcept = default;


////////////////////////////////////////////////////////////
std::optional<Music> Music::tryOpenFromInputSoundFile(std::optional<InputSoundFile>&& optFile, const char* errorContext)
{
    if (!optFile.has_value())
    {
        priv::err() << "Failed to open music from " << errorContext << priv::errEndl;
        return std::nullopt;
    }

    return std::make_optional<Music>(priv::PassKey<Music>{}, SFML_MOVE(*optFile));
}


////////////////////////////////////////////////////////////
std::optional<Music> Music::openFromFile(const std::filesystem::path& filename)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromFile(filename), "file");
}


////////////////////////////////////////////////////////////
std::optional<Music> Music::openFromMemory(const void* data, std::size_t sizeInBytes)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromMemory(data, sizeInBytes), "memory");
}


////////////////////////////////////////////////////////////
std::optional<Music> Music::openFromStream(InputStream& stream)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromStream(stream), "stream");
}


////////////////////////////////////////////////////////////
Time Music::getDuration() const
{
    return m_file->getDuration();
}


////////////////////////////////////////////////////////////
unsigned int Music::getChannelCount() const
{
    return m_file->getChannelCount();
}


////////////////////////////////////////////////////////////
unsigned int Music::getSampleRate() const
{
    return m_file->getSampleRate();
}


////////////////////////////////////////////////////////////
std::vector<SoundChannel> Music::getChannelMap() const
{
    return m_file->getChannelMap();
}


////////////////////////////////////////////////////////////
[[nodiscard]] std::uint64_t Music::getSampleCount() const
{
    return m_file->getSampleCount();
}


////////////////////////////////////////////////////////////
MusicStream Music::createStream(PlaybackDevice& playbackDevice)
{
    return MusicStream{playbackDevice, *this};
}


////////////////////////////////////////////////////////////
Music::Music(priv::PassKey<Music>&&, InputSoundFile&& file) : m_file(SFML_MOVE(file))
{
}

} // namespace sf
