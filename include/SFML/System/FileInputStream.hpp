#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Export.hpp"

#include "SFML/System/InputStream.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <cstdio> // TODO P2: try to remove


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
    /// \brief Open the file at `filename` for reading
    ///
    /// \return Stream on success, `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<FileInputStream> open(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Read up to `size` bytes into `data`, advancing the reading position
    ///
    /// \return Number of bytes actually read, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> read(void* data, base::SizeT size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Move the reading position to `position` (offset from the beginning)
    ///
    /// \return Position actually sought to, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> seek(base::SizeT position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Current reading position, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> tell() override;

    ////////////////////////////////////////////////////////////
    /// \brief Total number of bytes in the stream, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::SizeT> getSize() override;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Deleter that closes the wrapped stdio file
    ///
    ////////////////////////////////////////////////////////////
    struct FileCloser
    {
        void operator()(std::FILE* file);
    };

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ////////////////////////////////////////////////////////////
    explicit FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<std::FILE, FileCloser>&& file);

#ifdef SFML_SYSTEM_ANDROID
    ////////////////////////////////////////////////////////////
    /// \private
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
/// `InputStream` specialization that reads from a file on disk.
///
/// SFML resource classes can usually be loaded directly from a filename,
/// so this is mostly useful when writing custom algorithms over
/// `InputStream`.
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
