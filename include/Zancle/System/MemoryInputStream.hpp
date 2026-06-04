#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "Zancle/System/Export.hpp"

#include "Zancle/System/InputStream.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Implementation of input stream based on a memory chunk
///
////////////////////////////////////////////////////////////
class ZA_SYSTEM_API MemoryInputStream : public InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct from a pointer to `sizeInBytes` bytes of data
    ///
    ////////////////////////////////////////////////////////////
    MemoryInputStream(const void* data, zb::SizeT sizeInBytes);

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
    // Member data
    ////////////////////////////////////////////////////////////
    const unsigned char* m_data{};   //!< Pointer to the data in memory
    zb::SizeT          m_size{};   //!< Total size of the data
    zb::SizeT          m_offset{}; //!< Current reading position
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::MemoryInputStream
/// \ingroup system
///
/// `InputStream` specialization that wraps a chunk of in-memory data,
/// letting code written against `InputStream` operate on it.
///
/// SFML resource classes can usually be loaded directly from memory,
/// so this is mostly useful when writing custom algorithms over
/// `InputStream`.
///
/// Usage example:
/// \code
/// void process(InputStream& stream);
///
/// MemoryInputStream stream(thePtr, theSize);
/// process(stream);
/// \endcode
///
/// \see `InputStream`, `FileInputStream`
///
////////////////////////////////////////////////////////////
