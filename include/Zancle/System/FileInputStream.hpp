#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"

#include "Zancle/System/Export.hpp"

#include "Zancle/System/InputStream.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/UniquePtr.hpp"

#include <cstdio> // TODO P2: try to remove


#ifdef ZA_SYSTEM_ANDROID
namespace za::priv
{
class ZA_SYSTEM_API ResourceStream;
}
#endif


namespace za
{
class Path;

////////////////////////////////////////////////////////////
/// \brief Implementation of input stream based on a file
///
////////////////////////////////////////////////////////////
class ZA_SYSTEM_API FileInputStream : public InputStream
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
    /// \return Stream on success, `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static zb::Optional<FileInputStream> open(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Read up to `size` bytes into `data`, advancing the reading position
    ///
    /// \return Number of bytes actually read, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::Optional<zb::SizeT> read(void* data, zb::SizeT size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Move the reading position to `position` (offset from the beginning)
    ///
    /// \return Position actually sought to, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::Optional<zb::SizeT> seek(zb::SizeT position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Current reading position, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::Optional<zb::SizeT> tell() override;

    ////////////////////////////////////////////////////////////
    /// \brief Total number of bytes in the stream, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::Optional<zb::SizeT> getSize() override;

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
    explicit FileInputStream(zb::PassKey<FileInputStream>&&, zb::UniquePtr<std::FILE, FileCloser>&& file);

#ifdef ZA_SYSTEM_ANDROID
    ////////////////////////////////////////////////////////////
    /// \private
    ////////////////////////////////////////////////////////////
    explicit FileInputStream(zb::PassKey<FileInputStream>&&, zb::UniquePtr<priv::ResourceStream>&& androidFile);
#endif

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
#ifdef ZA_SYSTEM_ANDROID
    zb::UniquePtr<priv::ResourceStream> m_androidFile;
#endif

    zb::UniquePtr<std::FILE, FileCloser> m_file; //!< stdio file stream
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::FileInputStream
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
/// zb::Optional stream = za::FileInputStream::open("some_file.dat");
/// if (stream)
///    process(*stream);
/// \endcode
///
/// \see `InputStream`, `MemoryInputStream`
///
////////////////////////////////////////////////////////////
