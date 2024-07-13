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
#include <SFML/Audio/SoundFileFactory.hpp>
#include <SFML/Audio/SoundFileReaderFlac.hpp>
#include <SFML/Audio/SoundFileReaderMp3.hpp>
#include <SFML/Audio/SoundFileReaderOgg.hpp>
#include <SFML/Audio/SoundFileReaderWav.hpp>
#include <SFML/Audio/SoundFileWriterFlac.hpp>
#include <SFML/Audio/SoundFileWriterOgg.hpp>
#include <SFML/Audio/SoundFileWriterWav.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/MemoryInputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/PathUtils.hpp>

#include <unordered_map>


namespace
{
////////////////////////////////////////////////////////////
template <typename T>
using CreateFnPtr = sf::priv::UniquePtr<T> (*)();

using ReaderCheckFnPtr = bool (*)(sf::InputStream&);
using WriterCheckFnPtr = bool (*)(const sf::Path&);

using ReaderFactoryMap = std::unordered_map<CreateFnPtr<sf::SoundFileReader>, ReaderCheckFnPtr>;
using WriterFactoryMap = std::unordered_map<CreateFnPtr<sf::SoundFileWriter>, WriterCheckFnPtr>;


////////////////////////////////////////////////////////////
[[nodiscard]] ReaderFactoryMap& getReaderFactoryMap()
{
    // The map is pre-populated with default readers on construction
    static ReaderFactoryMap
        result{{&sf::priv::createReader<sf::priv::SoundFileReaderFlac>, &sf::priv::SoundFileReaderFlac::check},
               {&sf::priv::createReader<sf::priv::SoundFileReaderMp3>, &sf::priv::SoundFileReaderMp3::check},
               {&sf::priv::createReader<sf::priv::SoundFileReaderOgg>, &sf::priv::SoundFileReaderOgg::check},
               {&sf::priv::createReader<sf::priv::SoundFileReaderWav>, &sf::priv::SoundFileReaderWav::check}};

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard]] WriterFactoryMap& getWriterFactoryMap()
{
    // The map is pre-populated with default writers on construction
    static WriterFactoryMap
        result{{&sf::priv::createWriter<sf::priv::SoundFileWriterFlac>, &sf::priv::SoundFileWriterFlac::check},
               {&sf::priv::createWriter<sf::priv::SoundFileWriterOgg>, &sf::priv::SoundFileWriterOgg::check},
               {&sf::priv::createWriter<sf::priv::SoundFileWriterWav>, &sf::priv::SoundFileWriterWav::check}};

    return result;
}

} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
priv::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromFilename(const Path& filename)
{
    // Wrap the input file into a file stream
    auto stream = FileInputStream::open(filename);
    if (!stream)
    {
        priv::err() << "Failed to open sound file (couldn't open stream)\n" << priv::formatDebugPathInfo(filename);
        return nullptr;
    }

    // Test the filename in all the registered factories
    for (const auto& [fpCreate, fpCheck] : getReaderFactoryMap())
    {
        if (!stream->seek(0).hasValue())
        {
            priv::err() << "Failed to seek sound stream";
            return nullptr;
        }

        if (fpCheck(*stream))
            return fpCreate();
    }

    // No suitable reader found
    priv::err() << "Failed to open sound file (format not supported)\n" << priv::formatDebugPathInfo(filename);
    return nullptr;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromMemory(const void* data, std::size_t sizeInBytes)
{
    // Wrap the memory file into a file stream
    MemoryInputStream stream(data, sizeInBytes);

    // Test the stream for all the registered factories
    for (const auto& [fpCreate, fpCheck] : getReaderFactoryMap())
    {
        if (!stream.seek(0).hasValue())
        {
            priv::err() << "Failed to seek sound stream";
            return nullptr;
        }

        if (fpCheck(stream))
            return fpCreate();
    }

    // No suitable reader found
    priv::err() << "Failed to open sound file from memory (format not supported)";
    return nullptr;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromStream(InputStream& stream)
{
    // Test the stream for all the registered factories
    for (const auto& [fpCreate, fpCheck] : getReaderFactoryMap())
    {
        if (!stream.seek(0).hasValue())
        {
            priv::err() << "Failed to seek sound stream";
            return nullptr;
        }

        if (fpCheck(stream))
            return fpCreate();
    }

    // No suitable reader found
    priv::err() << "Failed to open sound file from stream (format not supported)";
    return nullptr;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<SoundFileWriter> SoundFileFactory::createWriterFromFilename(const Path& filename)
{
    // Test the filename in all the registered factories
    for (const auto& [fpCreate, fpCheck] : getWriterFactoryMap())
    {
        if (fpCheck(filename))
            return fpCreate();
    }

    // No suitable writer found
    priv::err() << "Failed to open sound file (format not supported)\n" << priv::formatDebugPathInfo(filename);
    return nullptr;
}


////////////////////////////////////////////////////////////
void SoundFileFactory::registerReaderImpl(CreateFnPtr<SoundFileReader> key, ReaderCheckFnPtr value)
{
    getReaderFactoryMap()[key] = value;
}


////////////////////////////////////////////////////////////
void SoundFileFactory::unregisterReaderImpl(CreateFnPtr<SoundFileReader> key)
{
    getReaderFactoryMap().erase(key);
}


////////////////////////////////////////////////////////////
bool SoundFileFactory::isReaderRegisteredImpl(CreateFnPtr<SoundFileReader> key)
{
    return getReaderFactoryMap().count(key) == 1;
}


////////////////////////////////////////////////////////////
void SoundFileFactory::registerWriterImpl(CreateFnPtr<SoundFileWriter> key, WriterCheckFnPtr value)
{
    getWriterFactoryMap()[key] = value;
}


////////////////////////////////////////////////////////////
void SoundFileFactory::unregisterWriterImpl(CreateFnPtr<SoundFileWriter> key)
{
    getWriterFactoryMap().erase(key);
}


////////////////////////////////////////////////////////////
bool SoundFileFactory::isWriterRegisteredImpl(CreateFnPtr<SoundFileWriter> key)
{
    return getWriterFactoryMap().count(key) == 1;
}

} // namespace sf
