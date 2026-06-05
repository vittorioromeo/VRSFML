// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/SoundFileFactory.hpp"

#include "Zancle/Audio/SoundFileReaderFlac.hpp"
#include "Zancle/Audio/SoundFileReaderMp3.hpp"
#include "Zancle/Audio/SoundFileReaderOgg.hpp"
#include "Zancle/Audio/SoundFileReaderWav.hpp"
#include "Zancle/Audio/SoundFileWriterFlac.hpp"
#include "Zancle/Audio/SoundFileWriterOgg.hpp"
#include "Zancle/Audio/SoundFileWriterWav.hpp"

#include "Zancle/Err/Err.hpp"
#include "Zancle/IO/FileInputStream.hpp"
#include "Zancle/IO/MemoryInputStream.hpp"
#include "Zancle/IO/Path.hpp"
#include "Zancle/IO/PathUtils.hpp"

#include "Zancle/Container/AnkerlUnorderedDense.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"


namespace
{
////////////////////////////////////////////////////////////
template <typename T>
using CreateFnPtr = za::UniquePtr<T> (*)();

using ReaderCheckFnPtr = bool (*)(za::InputStream&);
using WriterCheckFnPtr = bool (*)(const za::Path&);

using ReaderFactoryMap = ankerl::unordered_dense::map<CreateFnPtr<za::SoundFileReader>, ReaderCheckFnPtr>;
using WriterFactoryMap = ankerl::unordered_dense::map<CreateFnPtr<za::SoundFileWriter>, WriterCheckFnPtr>;


////////////////////////////////////////////////////////////
[[nodiscard]] ReaderFactoryMap& getReaderFactoryMap()
{
    // The map is pre-populated with default readers on construction
    static ReaderFactoryMap
        result{{&za::priv::createReader<za::priv::SoundFileReaderFlac>, &za::priv::SoundFileReaderFlac::check},
               {&za::priv::createReader<za::priv::SoundFileReaderMp3>, &za::priv::SoundFileReaderMp3::check},
               {&za::priv::createReader<za::priv::SoundFileReaderOgg>, &za::priv::SoundFileReaderOgg::check},
               {&za::priv::createReader<za::priv::SoundFileReaderWav>, &za::priv::SoundFileReaderWav::check}};

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard]] WriterFactoryMap& getWriterFactoryMap()
{
    // The map is pre-populated with default writers on construction
    static WriterFactoryMap
        result{{&za::priv::createWriter<za::priv::SoundFileWriterFlac>, &za::priv::SoundFileWriterFlac::check},
               {&za::priv::createWriter<za::priv::SoundFileWriterOgg>, &za::priv::SoundFileWriterOgg::check},
               {&za::priv::createWriter<za::priv::SoundFileWriterWav>, &za::priv::SoundFileWriterWav::check}};

    return result;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
za::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromFilename(const Path& filename)
{
    za::UniquePtr<SoundFileReader> result; // Use a single local variable for NRVO

    auto stream = FileInputStream::open(filename);
    if (!stream.hasValue())
    {
        priv::errMsg("Failed to open sound file (couldn't open stream)\n{}", priv::PathDebugFormatter{filename});
        return result; // Null
    }

    result = createReaderFromStream(*stream);
    if (result == nullptr)
        priv::errMsg("Sound file: {}", priv::PathDebugFormatter{filename});

    return result;
}


////////////////////////////////////////////////////////////
za::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromMemory(const void* data, za::SizeT sizeInBytes)
{
    MemoryInputStream stream(data, sizeInBytes);
    return createReaderFromStream(stream);
}


////////////////////////////////////////////////////////////
za::UniquePtr<SoundFileReader> SoundFileFactory::createReaderFromStream(InputStream& stream)
{
    // Test the stream for all the registered factories
    for (const auto& [fpCreate, fpCheck] : getReaderFactoryMap())
    {
        if (!stream.seek(0).hasValue())
        {
            priv::errMsg("Failed to seek sound stream");
            return nullptr;
        }

        if (fpCheck(stream))
            return fpCreate();
    }

    // No suitable reader found
    priv::errMsg("Failed to open sound file from stream (format not supported)");
    return nullptr;
}


////////////////////////////////////////////////////////////
za::UniquePtr<SoundFileWriter> SoundFileFactory::createWriterFromFilename(const Path& filename)
{
    // Test the filename in all the registered factories
    for (const auto& [fpCreate, fpCheck] : getWriterFactoryMap())
    {
        if (fpCheck(filename))
            return fpCreate();
    }

    // No suitable writer found
    priv::errMsg("Failed to open sound file (format not supported)\n{}", priv::PathDebugFormatter{filename});
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

} // namespace za
