#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputStream;
class Path;
class SoundFileReader;
class SoundFileWriter;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Manages and instantiates sound file readers and writers
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundFileFactory
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Register a new reader
    ///
    /// \see `unregisterReader`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void registerReader();

    ////////////////////////////////////////////////////////////
    /// \brief Unregister a reader
    ///
    /// \see `registerReader`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void unregisterReader();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a reader is registered
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static bool isReaderRegistered();

    ////////////////////////////////////////////////////////////
    /// \brief Register a new writer
    ///
    /// \see `unregisterWriter`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void registerWriter();

    ////////////////////////////////////////////////////////////
    /// \brief Unregister a writer
    ///
    /// \see `registerWriter`
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static void unregisterWriter();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a writer is registered
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] static bool isWriterRegistered();

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right reader for the given file on disk
    ///
    /// \param filename Path of the sound file
    ///
    /// \return A new sound file reader that can read the given file, or null if no reader can handle it
    ///
    /// \see `createReaderFromMemory`, `createReaderFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<SoundFileReader> createReaderFromFilename(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right codec for the given file in memory
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Total size of the file data, in bytes
    ///
    /// \return A new sound file codec that can read the given file, or null if no codec can handle it
    ///
    /// \see `createReaderFromFilename`, `createReaderFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<SoundFileReader> createReaderFromMemory(const void* data, base::SizeT sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right codec for the given file in stream
    ///
    /// \param stream Source stream to read from
    ///
    /// \return A new sound file codec that can read the given file, or null if no codec can handle it
    ///
    /// \see `createReaderFromFilename`, `createReaderFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<SoundFileReader> createReaderFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Instantiate the right writer for the given file on disk
    ///
    /// \param filename Path of the sound file
    ///
    /// \return A new sound file writer that can write given file, or null if no writer can handle it
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<SoundFileWriter> createWriterFromFilename(const Path& filename);

private:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    template <typename T>
    using CreateFnPtr = base::UniquePtr<T> (*)();

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

} // namespace sf

#include "SFML/Audio/SoundFileFactory.inl"


////////////////////////////////////////////////////////////
/// \class sf::SoundFileFactory
/// \ingroup audio
///
/// This class is where all the sound file readers and writers are
/// registered. You should normally only need to use its registration
/// and unregistration functions; readers/writers creation and manipulation
/// are wrapped into the higher-level classes `sf::InputSoundFile` and
/// `sf::OutputSoundFile`.
///
/// To register a new reader (writer) use the `sf::SoundFileFactory::registerReader`
/// (`registerWriter`) static function. You don't have to call the `unregisterReader`
/// (`unregisterWriter`) function, unless you want to unregister a format before your
/// application ends (typically, when a plugin is unloaded).
///
/// Usage example:
/// \code
/// sf::SoundFileFactory::registerReader<MySoundFileReader>();
/// SFML_BASE_ASSERT(sf::SoundFileFactory::isReaderRegistered<MySoundFileReader>());
///
/// sf::SoundFileFactory::registerWriter<MySoundFileWriter>();
/// SFML_BASE_ASSERT(sf::SoundFileFactory::isWriterRegistered<MySoundFileWriter>());
/// \endcode
///
/// \see `sf::InputSoundFile`, `sf::OutputSoundFile`, `sf::SoundFileReader`, `sf::SoundFileWriter`
///
////////////////////////////////////////////////////////////
