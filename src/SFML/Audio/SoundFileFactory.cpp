// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileFactory.hpp"
#include "SFML/Audio/SoundFileReaderFlac.hpp"
#include "SFML/Audio/SoundFileReaderMp3.hpp"
#include "SFML/Audio/SoundFileReaderOgg.hpp"
#include "SFML/Audio/SoundFileReaderWav.hpp"
#include "SFML/Audio/SoundFileWriterFlac.hpp"
#include "SFML/Audio/SoundFileWriterOgg.hpp"
#include "SFML/Audio/SoundFileWriterWav.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/MemoryInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"


namespace
{
////////////////////////////////////////////////////////////
template <typename T>
using CreateFnPtr = sf::base::UniquePtr<T> (*)();

using ReaderCheckFnPtr = bool (*)(sf::InputStream&);
using WriterCheckFnPtr = bool (*)(const sf::Path&);

using ReaderFactoryMap = ankerl::unordered_dense::map<CreateFnPtr<sf::SoundFileReader>, ReaderCheckFnPtr>;
using WriterFactoryMap = ankerl::unordered_dense::map<CreateFnPtr<sf::SoundFileWriter>, WriterCheckFnPtr>;


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
base::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromFilename(const Path& filename)
{
    // Wrap the input file into a file stream
    auto stream = FileInputStream::open(filename);
    if (!stream.hasValue())
    {
        priv::err() << "Failed to open sound file (couldn't open stream)\n" << priv::PathDebugFormatter{filename};
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
    priv::err() << "Failed to open sound file (format not supported)\n" << priv::PathDebugFormatter{filename};
    return nullptr;
}


////////////////////////////////////////////////////////////
base::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromMemory(const void* data, base::SizeT sizeInBytes)
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
base::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromStream(InputStream& stream)
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
base::UniquePtr<SoundFileWriter> SoundFileFactory::createWriterFromFilename(const Path& filename)
{
    // Test the filename in all the registered factories
    for (const auto& [fpCreate, fpCheck] : getWriterFactoryMap())
    {
        if (fpCheck(filename))
            return fpCreate();
    }

    // No suitable writer found
    priv::err() << "Failed to open sound file (format not supported)\n" << priv::PathDebugFormatter{filename};
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
