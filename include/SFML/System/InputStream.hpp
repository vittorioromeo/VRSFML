#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Export.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Abstract class for custom file input streams
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~InputStream() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Read up to `size` bytes into `data`, advancing the reading position
    ///
    /// \return Number of bytes actually read, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<base::SizeT> read(void* data, base::SizeT size) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Move the reading position to `position` (offset from the beginning)
    ///
    /// \return Position actually sought to, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<base::SizeT> seek(base::SizeT position) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Current reading position, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<base::SizeT> tell() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Total number of bytes in the stream, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<base::SizeT> getSize() = 0;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::InputStream
/// \ingroup system
///
/// This class allows users to define their own file input sources
/// from which SFML can load resources.
///
/// SFML resource classes like `sf::Texture` and
/// `sf::SoundBuffer` provide `loadFromFile` and `loadFromMemory` functions,
/// which read data from conventional sources. However, if you
/// have data coming from a different source (over a network,
/// embedded, encrypted, compressed, etc) you can derive your
/// own class from `sf::InputStream` and load SFML resources with
/// their `loadFromStream` function.
///
/// Usage example:
/// \code
/// // custom stream class that reads from inside a zip file
/// class ZipStream : public sf::InputStream
/// {
/// public:
///
///     ZipStream(const std::string& archive);
///
///     [[nodiscard]] bool open(const Path& filename);
///
///     [[nodiscard]] sf::base::Optional<base::SizeT> read(void* data, base::SizeT size);
///
///     [[nodiscard]] sf::base::Optional<base::SizeT> seek(base::SizeT position);
///
///     [[nodiscard]] sf::base::Optional<base::SizeT> tell();
///
///     sf::base::Optional<base::SizeT> getSize();
///
/// private:
///
///     ...
/// };
///
/// // now you can load textures...
/// ZipStream stream("resources.zip");
///
/// if (!stream.open("images/img.png"))
/// {
///     // Handle error...
/// }
///
/// const auto texture = sf::Texture::loadFromStream(stream).value();
///
/// // musics...
/// sf::Music music;
/// ZipStream stream("resources.zip");
///
/// if (!stream.open("musics/msc.ogg"))
/// {
///     // Handle error...
/// }
///
/// if (!music.openFromStream(stream))
/// {
///     // Handle error...
/// }
///
/// // etc.
/// \endcode
///
/// \see `FileInputStream`, `MemoryInputStream`
///
////////////////////////////////////////////////////////////
