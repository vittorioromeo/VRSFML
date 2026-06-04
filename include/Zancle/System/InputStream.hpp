#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"
#include "Zancle/System/Export.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Abstract class for custom file input streams
///
////////////////////////////////////////////////////////////
class ZA_SYSTEM_API InputStream
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
    /// \return Number of bytes actually read, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual zb::Optional<zb::SizeT> read(void* data, zb::SizeT size) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Move the reading position to `position` (offset from the beginning)
    ///
    /// \return Position actually sought to, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual zb::Optional<zb::SizeT> seek(zb::SizeT position) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Current reading position, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual zb::Optional<zb::SizeT> tell() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Total number of bytes in the stream, or `zb::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual zb::Optional<zb::SizeT> getSize() = 0;
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::InputStream
/// \ingroup system
///
/// This class allows users to define their own file input sources
/// from which SFML can load resources.
///
/// SFML resource classes like `za::Texture` and
/// `za::SoundBuffer` provide `loadFromFile` and `loadFromMemory` functions,
/// which read data from conventional sources. However, if you
/// have data coming from a different source (over a network,
/// embedded, encrypted, compressed, etc) you can derive your
/// own class from `za::InputStream` and load SFML resources with
/// their `loadFromStream` function.
///
/// Usage example:
/// \code
/// // custom stream class that reads from inside a zip file
/// class ZipStream : public za::InputStream
/// {
/// public:
///
///     ZipStream(const std::string& archive);
///
///     [[nodiscard]] bool open(const Path& filename);
///
///     [[nodiscard]] zb::Optional<zb::SizeT> read(void* data, zb::SizeT size);
///
///     [[nodiscard]] zb::Optional<zb::SizeT> seek(zb::SizeT position);
///
///     [[nodiscard]] zb::Optional<zb::SizeT> tell();
///
///     zb::Optional<zb::SizeT> getSize();
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
/// const auto texture = za::Texture::loadFromStream(stream).value();
///
/// // musics...
/// za::Music music;
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
