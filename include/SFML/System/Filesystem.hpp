////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>


namespace sf
{
class FilesystemPath
{
public:
    using value_type  = wchar_t;
    using string_type = std::basic_string<wchar_t>;

    FilesystemPath();
    ~FilesystemPath();

    FilesystemPath(const std::string& str);
    FilesystemPath(std::string_view str);
    FilesystemPath(const char* str);

    FilesystemPath& operator=(const FilesystemPath& rhs);

    const value_type* c_str() const;
    std::string       string() const;
    FilesystemPath    extension() const;
    FilesystemPath    filename() const;

    bool operator==(const char* str) const;

    FilesystemPath& operator/=(const FilesystemPath& path);

    bool empty() const;

    friend FilesystemPath operator/(const FilesystemPath& path, const char* str);
    friend FilesystemPath operator/(const char* str, const FilesystemPath& path);
    friend FilesystemPath operator/(const FilesystemPath& pathLhs, const FilesystemPath& pathRhs);
    friend std::ostream&  operator<<(std::ostream& os, const FilesystemPath& path);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    FilesystemPath(std::unique_ptr<Impl>&&);
};

FilesystemPath filesystemTempDirectoryPath();
FilesystemPath filesystemAbsolute(const FilesystemPath& path);
bool           filesystemRemove(const FilesystemPath& path);

} // namespace sf


////////////////////////////////////////////////////////////
/// \fn sf::err
/// \ingroup system
///
/// By default, sf::err() outputs to the same location as std::cerr,
/// (-> the stderr descriptor) which is the console if there's
/// one available.
///
/// It is a standard std::ostream instance, so it supports all the
/// insertion operations defined by the STL
/// (operator <<, manipulators, etc.).
///
/// sf::err() can be redirected to write to another output, independently
/// of std::cerr, by using the rdbuf() function provided by the
/// std::ostream class.
///
/// Example:
/// \code
/// // Redirect to a file
/// std::ofstream file("sfml-log.txt");
/// std::streambuf* previous = sf::err().rdbuf(file.rdbuf());
///
/// // Redirect to nothing
/// sf::err().rdbuf(nullptr);
///
/// // Restore the original output
/// sf::err().rdbuf(previous);
/// \endcode
///
/// \return Reference to std::ostream representing the SFML error stream
///
////////////////////////////////////////////////////////////
