#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/StringView.hpp"

#include <iosfwd>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Represents a path in the filesystem.
/// \ingroup system
///
/// This class provides a cross-platform way to represent and manipulate filesystem paths.
/// It uses PImpl to wrap `std::filesystem::path`, avoiding exposure of `<filesystem>` in SFML headers.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_SYSTEM_API Path
{
public:
#if defined(SFML_SYSTEM_EMSCRIPTEN) || defined(SFML_SYSTEM_LINUX_OR_BSD)
    using value_type = char;
#else
    using value_type = wchar_t;
#endif

    ////////////////////////////////////////////////////////////
    /// \brief Gets the path to the system's temporary directory
    ///
    /// \return Path representing the temporary directory.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Path tempDirectoryPath();

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Constructs an empty path.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Path();

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a path from a source
    ///
    /// \tparam T Type of the source, typically a string type like `std::string`, `const char*`,
    ///           `std::wstring`, `const wchar_t*`, or `std::filesystem::path`.
    ///
    /// \param source The source to construct the path from.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] Path(const T& source);

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a path from a null-terminated string source
    ///
    /// \tparam T Character type of the source string (e.g., `char`, `wchar_t`).
    /// \param source The null-terminated string to construct the path from.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] Path(const T* source);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Path();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Path(const Path&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment operator
    ///
    ////////////////////////////////////////////////////////////
    Path& operator=(const Path&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Path(Path&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    Path& operator=(Path&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Returns the filename component of the path.
    ///
    /// Example: `/foo/bar.txt` -> `bar.txt`
    /// \return A new Path object representing the filename.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Path filename() const;

    ////////////////////////////////////////////////////////////
    /// \brief Returns the extension of the filename component.
    ///
    /// Example: `/foo/bar.txt` -> `.txt`
    /// \return A new Path object representing the extension, or an empty Path if no extension.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Path extension() const;

    ////////////////////////////////////////////////////////////
    /// \brief Returns the absolute version of the path.
    ///
    /// \return A new Path object representing the absolute path.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Path absolute() const;

    ////////////////////////////////////////////////////////////
    /// \brief Returns a C-style string representation of the path
    ///
    /// The returned pointer is to the internal representation native to the OS
    /// (e.g., `const wchar_t*` on Windows, `const char*` on POSIX).
    /// The lifetime of the pointed-to string is managed by the `Path` object.
    ///
    /// \return Pointer to a null-terminated string.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const value_type* c_str() const; // NOLINT(readability-identifier-naming)

    ////////////////////////////////////////////////////////////
    /// \brief Returns a C-style `const char*` representation of the path (UTF-8 encoded)
    ///
    /// This method may involve a conversion if the native path encoding is not UTF-8.
    /// The lifetime of the pointed-to string is managed by the `Path` object but is
    /// only guaranteed to be valid until the next non-const method call on this `Path` object.
    ///
    /// \return Pointer to a null-terminated UTF-8 encoded C-string.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* toCharPtr() const;

    ////////////////////////////////////////////////////////////
    /// \brief Converts the path to a string of type `T`
    ///
    /// Supported `T` types include `std::string`, `std::wstring`, `std::u8string`,
    /// `std::u32string`, and `std::filesystem::path`.
    ///
    /// \return The path converted to the specified string type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    T to() const;

    ////////////////////////////////////////////////////////////
    /// \brief Removes the file or empty directory identified by the path
    ///
    /// \return `true` if removal succeeds, `false` otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool remove() const;

    ////////////////////////////////////////////////////////////
    /// \brief Checks if the path is empty
    ///
    /// \return `true` if the path is empty, `false` otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool empty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Checks if the path refers to an existing file or directory
    ///
    /// \return `true` if the path exists, `false` otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool exists() const;

    ////////////////////////////////////////////////////////////
    /// \brief Checks if the path's extension matches a given string
    ///
    /// \param str The string to compare the extension against.
    ///
    /// \return `true` if the extension matches, `false` otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool extensionIs(base::StringView str) const;

    ////////////////////////////////////////////////////////////
    /// \brief Appends another path to this one
    ///
    /// \param rhs The path to append.
    ///
    /// \return Reference to this path after appending.
    ///
    ////////////////////////////////////////////////////////////
    Path& operator/=(const Path& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Concatenates two paths
    ///
    /// \param lhs The left-hand side path.
    /// \param rhs The right-hand side path to append.
    ///
    /// \return A new `Path` object representing the concatenated path.
    ///
    ////////////////////////////////////////////////////////////
    friend Path operator/(const Path& lhs, const Path& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Outputs the path to a stream
    ///
    /// \param os The output stream.
    /// \param path The path to output.
    ///
    /// \return The output stream.
    ///
    ////////////////////////////////////////////////////////////
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a `Path` from `str` and compares it for equality with the current path
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool operator==(const T* str) const;

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a `Path` from `str` and compares it for inequality with the current path
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool operator!=(const T* str) const;

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a `Path` from `str` and compares it for equality with the current path
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool operator==(const T& str) const;

    ////////////////////////////////////////////////////////////
    /// \brief Constructs a `Path` from `str` and compares it for inequality with the current path
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool operator!=(const T& str) const;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Private constructor for internal use
    ///
    /// \param fsPath The filesystem path to wrap.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Path(int, const void* fsPath);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl;
};


} // namespace sf
