#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Export.hpp"

#include "SFML/System/InputStream.hpp"

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
    /// \brief Construct the stream from its data
    ///
    /// \param data        Pointer to the data in memory
    /// \param sizeInBytes Size of the data, in bytes
    ///
    ////////////////////////////////////////////////////////////
    MemoryInputStream(const void* data, base::SizeT sizeInBytes);

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
/// This class is a specialization of `InputStream` that
/// reads from data in memory.
///
/// It wraps a memory chunk in the common `InputStream` interface
/// and therefore allows to use generic classes or functions
/// that accept such a stream, with content already loaded in memory.
///
/// In addition to the virtual functions inherited from
/// `InputStream`, `MemoryInputStream` adds a function to
/// specify the pointer and size of the data in memory.
///
/// SFML resource classes can usually be loaded directly from
/// memory, so this class shouldn't be useful to you unless
/// you create your own algorithms that operate on an InputStream.
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
