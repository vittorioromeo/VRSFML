#pragma once

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/FmtToString.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf::testing
{
////////////////////////////////////////////////////////////
/// \brief Generate a unique temporary file path
///
/// Each call returns a new path under the system temp directory.
///
////////////////////////////////////////////////////////////
inline Path getTemporaryFilePath()
{
    static int counter = 0;

    const auto tmp = Path::getTempDirectory();
    SFML_BASE_ASSERT(tmp && "Failed to obtain temp directory");

    return *tmp / Path(base::fmtToString("sfmltemp_{}.tmp", counter++));
}


////////////////////////////////////////////////////////////
/// \brief RAII temporary file for tests
///
/// Creates a file on disk with the given contents (if any),
/// and removes it in the destructor.
///
////////////////////////////////////////////////////////////
class TemporaryFile
{
public:
    /// Reserve a unique path without creating any file on disk.
    explicit TemporaryFile() : m_path(getTemporaryFilePath())
    {
    }

    /// Create a temporary file containing \a contents.
    explicit TemporaryFile(base::StringView contents) : m_path(getTemporaryFilePath())
    {
        auto optFile = OutFile::open(m_path);
        SFML_BASE_ASSERT(optFile.hasValue() && "Failed to open temporary file for writing");

        [[maybe_unused]] const bool wrote = optFile->write(contents.data(), contents.size());
        SFML_BASE_ASSERT(wrote && "Failed to write temporary file contents");

        // Destructor of `optFile` closes the file when this body returns.
    }

    ~TemporaryFile()
    {
        if (m_path.exists())
        {
            [[maybe_unused]] const bool removed = m_path.removeFromDisk();
            SFML_BASE_ASSERT(removed && "m_path failed to be removed from filesystem");
        }
    }

    TemporaryFile(const TemporaryFile&)            = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    [[nodiscard]] const Path& getPath() const
    {
        return m_path;
    }

private:
    Path m_path;
};

} // namespace sf::testing
