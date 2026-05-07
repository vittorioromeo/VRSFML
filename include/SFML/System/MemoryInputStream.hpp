#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "SFML/System/Export.hpp"

#include "SFML/System/InputStream.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Implementation of input stream based on a memory chunk
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API MemoryInputStream : public InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct from a pointer to `sizeInBytes` bytes of data
    ///
    ////////////////////////////////////////////////////////////
    MemoryInputStream(const void* data, base::SizeT sizeInBytes);

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
    // Member data
    ////////////////////////////////////////////////////////////
    const unsigned char* m_data{};   //!< Pointer to the data in memory
    base::SizeT          m_size{};   //!< Total size of the data
    base::SizeT          m_offset{}; //!< Current reading position
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::MemoryInputStream
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
