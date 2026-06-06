#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class InputStream;
class Path;
class SoundFileReader;
class SoundFileWriter;
} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
template <typename T>
za::UniquePtr<SoundFileReader> createReader()
{
    return za::makeUnique<T>();
}


////////////////////////////////////////////////////////////
template <typename T>
za::UniquePtr<SoundFileWriter> createWriter()
{
    return za::makeUnique<T>();
}

} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Manages and instantiates sound file readers and writers
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API SoundFileFactory
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Register a new reader
    ///
    /// \see `unregisterReader`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void registerReader()
    {
        registerReaderImpl(&priv::createReader<T>, &T::check);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Unregister a reader
    ///
    /// \see `registerReader`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void unregisterReader()
    {
        unregisterReaderImpl(&priv::createReader<T>);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a reader is registered
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static bool isReaderRegistered()
    {
        return isReaderRegisteredImpl(&priv::createReader<T>);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Register a new writer
    ///
    /// \see `unregisterWriter`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void registerWriter()
    {
        registerWriterImpl(&priv::createWriter<T>, &T::check);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Unregister a writer
    ///
    /// \see `registerWriter`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void unregisterWriter()
    {
        unregisterWriterImpl(&priv::createWriter<T>);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a writer is registered
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static bool isWriterRegistered()
    {
        return isWriterRegisteredImpl(&priv::createWriter<T>);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right reader for the given file on disk
    ///
    /// \param filename Path of the sound file
    ///
    /// \return A new sound file reader that can read the given file, or `nullptr` if no reader can handle it
    ///
    /// \see `createReaderFromMemory`, `createReaderFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::UniquePtr<SoundFileReader> createReaderFromFilename(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right codec for the given file in memory
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Total size of the file data, in bytes
    ///
    /// \return A new sound file codec that can read the given file, or `nullptr` if no codec can handle it
    ///
    /// \see `createReaderFromFilename`, `createReaderFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::UniquePtr<SoundFileReader> createReaderFromMemory(const void* data, za::SizeT sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right codec for the given file in stream
    ///
    /// \param stream Source stream to read from
    ///
    /// \return A new sound file codec that can read the given file, or `nullptr` if no codec can handle it
    ///
    /// \see `createReaderFromFilename`, `createReaderFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::UniquePtr<SoundFileReader> createReaderFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right writer for the given file on disk
    ///
    /// \param filename Path of the sound file
    ///
    /// \return A new sound file writer that can write given file, or `nullptr` if no writer can handle it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::UniquePtr<SoundFileWriter> createWriterFromFilename(const Path& filename);

private:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    template <typename T>
    using CreateFnPtr = za::UniquePtr<T> (*)();

    using ReaderCheckFnPtr = bool (*)(InputStream&);
    using WriterCheckFnPtr = bool (*)(const Path&);

    ////////////////////////////////////////////////////////////
    // Static member functions
    ////////////////////////////////////////////////////////////
    static void               registerReaderImpl(CreateFnPtr<SoundFileReader> key, ReaderCheckFnPtr value);
    static void               unregisterReaderImpl(CreateFnPtr<SoundFileReader> key);
    [[nodiscard]] static bool isReaderRegisteredImpl(CreateFnPtr<SoundFileReader> key);

    static void               registerWriterImpl(CreateFnPtr<SoundFileWriter> key, WriterCheckFnPtr value);
    static void               unregisterWriterImpl(CreateFnPtr<SoundFileWriter> key);
    [[nodiscard]] static bool isWriterRegisteredImpl(CreateFnPtr<SoundFileWriter> key);
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::SoundFileFactory
/// \ingroup audio
///
/// This class is where all the sound file readers and writers are
/// registered. You should normally only need to use its registration
/// and unregistration functions; readers/writers creation and manipulation
/// are wrapped into the higher-level classes `za::InputSoundFile` and
/// `za::OutputSoundFile`.
///
/// To register a new reader (writer) use the `za::SoundFileFactory::registerReader`
/// (`registerWriter`) static function. You don't have to call the `unregisterReader`
/// (`unregisterWriter`) function, unless you want to unregister a format before your
/// application ends (typically, when a plugin is unloaded).
///
/// Usage example:
/// \code
/// za::SoundFileFactory::registerReader<MySoundFileReader>();
/// ZA_ASSERT(za::SoundFileFactory::isReaderRegistered<MySoundFileReader>());
///
/// za::SoundFileFactory::registerWriter<MySoundFileWriter>();
/// ZA_ASSERT(za::SoundFileFactory::isWriterRegistered<MySoundFileWriter>());
/// \endcode
///
/// \see `za::InputSoundFile`, `za::OutputSoundFile`, `za::SoundFileReader`, `za::SoundFileWriter`
///
////////////////////////////////////////////////////////////
