#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Export.hpp"

#include "SFML/System/InputStream.hpp"

#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <cstdio> // TODO P1: try to remove

#ifdef SFML_SYSTEM_ANDROID
namespace sf::priv
{
class SFML_SYSTEM_API ResourceStream;
}
#endif


namespace sf
{
class Path;

////////////////////////////////////////////////////////////
/// \brief Implementation of input stream based on a file
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API FileInputStream : public InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ///
    ////////////////////////////////////////////////////////////
    ~FileInputStream() override;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    FileInputStream(const FileInputStream&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    FileInputStream& operator=(const FileInputStream&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    FileInputStream(FileInputStream&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    FileInputStream& operator=(FileInputStream&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Open the stream from a file path
    ///
    /// \param filename Name of the file to open
    ///
    /// \return File input stream on success, `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<FileInputStream> open(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Read data from the stream
    ///
    /// After reading, the stream's reading position must be
    /// advanced by the amount of bytes read.
    ///
    /// \param data Buffer where to copy the read data
    /// \param size Desired number of bytes to read
    ///
    /// \return The number of bytes actually read, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> read(void* data, base::SizeT size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current reading position
    ///
    /// \param position The position to seek to, from the beginning
    ///
    /// \return The position actually sought to, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> seek(base::SizeT position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current reading position in the stream
    ///
    /// \return The current position, or `base::nullOpt` on error.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> tell() override;

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the stream
    ///
    /// \return The total number of bytes available in the stream, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> getSize() override;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Deleter for stdio file stream that closes the file stream
    ///
    ////////////////////////////////////////////////////////////
    struct FileCloser
    {
        void operator()(std::FILE* file);
    };

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct from file
    ///
    ////////////////////////////////////////////////////////////
    explicit FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<std::FILE, FileCloser>&& file);

#ifdef SFML_SYSTEM_ANDROID
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct from resource stream
    ///
    ////////////////////////////////////////////////////////////
    explicit FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<priv::ResourceStream>&& androidFile);
#endif

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_ANDROID
    base::UniquePtr<priv::ResourceStream> m_androidFile;
#endif

    base::UniquePtr<std::FILE, FileCloser> m_file; //!< stdio file stream
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::FileInputStream
/// \ingroup system
///
/// This class is a specialization of `InputStream` that
/// reads from a file on disk.
///
/// It wraps a file in the common `InputStream` interface
/// and therefore allows to use generic classes or functions
/// that accept such a stream, with a file on disk as the data
/// source.
///
/// In addition to the virtual functions inherited from
/// `InputStream`, `FileInputStream` adds a function to
/// specify the file to open.
///
/// SFML resource classes can usually be loaded directly from
/// a filename, so this class shouldn't be useful to you unless
/// you create your own algorithms that operate on an InputStream.
///
/// Usage example:
/// \code
/// void process(InputStream& stream);
///
/// sf::base::Optional stream = sf::FileInputStream::open("some_file.dat");
/// if (stream)
///    process(*stream);
/// \endcode
///
/// \see `InputStream`, `MemoryInputStream`
///
////////////////////////////////////////////////////////////
