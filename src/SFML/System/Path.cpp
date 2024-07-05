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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/Path.hpp>

#include <filesystem>


namespace
{
////////////////////////////////////////////////////////////
const void* asVoidPtr(const std::filesystem::path& path)
{
    return &path;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Path::Impl
{
    std::filesystem::path fsPath;
};


////////////////////////////////////////////////////////////
Path Path::tempDirectoryPath()
{
    return Path{0, asVoidPtr(std::filesystem::temp_directory_path())};
}


////////////////////////////////////////////////////////////
Path::Path() = default;


////////////////////////////////////////////////////////////
Path::Path(const char* path) : m_impl(path)
{
}


////////////////////////////////////////////////////////////
Path::Path(const std::string& path) : m_impl(path)
{
}


////////////////////////////////////////////////////////////
Path::Path(std::string_view path) : m_impl(path)
{
}


////////////////////////////////////////////////////////////
Path::Path(int, const void* fsPath) : m_impl(*static_cast<const std::filesystem::path*>(fsPath))
{
}


////////////////////////////////////////////////////////////
Path::~Path() = default;


////////////////////////////////////////////////////////////
Path::Path(const Path&) = default;


////////////////////////////////////////////////////////////
Path& Path::operator=(const Path&) = default;


////////////////////////////////////////////////////////////
Path::Path(Path&&) noexcept = default;


////////////////////////////////////////////////////////////
Path& Path::operator=(Path&&) noexcept = default;


////////////////////////////////////////////////////////////
Path Path::filename() const
{
    return Path{0, asVoidPtr(m_impl->fsPath.filename())};
}


////////////////////////////////////////////////////////////
Path Path::extension() const
{
    return Path{0, asVoidPtr(m_impl->fsPath.extension())};
}


////////////////////////////////////////////////////////////
const wchar_t* Path::c_str() const
{
    return m_impl->fsPath.c_str();
}


////////////////////////////////////////////////////////////
std::string Path::string() const
{
    return m_impl->fsPath.string();
}


////////////////////////////////////////////////////////////
Path::operator std::string() const
{
    return m_impl->fsPath.string();
}


////////////////////////////////////////////////////////////
bool Path::remove() const
{
    return std::filesystem::remove(m_impl->fsPath);
}


////////////////////////////////////////////////////////////
bool Path::empty() const
{
    return m_impl->fsPath.empty();
}


////////////////////////////////////////////////////////////
bool Path::exists() const
{
    return std::filesystem::exists(m_impl->fsPath);
}


////////////////////////////////////////////////////////////
Path& Path::operator/=(const Path& rhs)
{
    m_impl->fsPath /= rhs.m_impl->fsPath;
    return *this;
}


////////////////////////////////////////////////////////////
Path operator/(const Path& lhs, const Path& rhs)
{
    return Path{0, asVoidPtr(lhs.m_impl->fsPath / rhs.m_impl->fsPath)};
}


////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const Path& path)
{
    return os << path.m_impl->fsPath;
}


////////////////////////////////////////////////////////////
bool operator==(const Path& path, const char* str)
{
    return path.m_impl->fsPath == str;
}


////////////////////////////////////////////////////////////
bool operator!=(const Path& path, const char* str)
{
    return path.m_impl->fsPath != str;
}


////////////////////////////////////////////////////////////
bool operator==(const Path& path, std::string_view str)
{
    return path.m_impl->fsPath == str;
}


////////////////////////////////////////////////////////////
bool operator!=(const Path& path, std::string_view str)
{
    return path.m_impl->fsPath != str;
}


////////////////////////////////////////////////////////////
bool operator==(const Path& path, const std::string& str)
{
    return path.m_impl->fsPath == str;
}


////////////////////////////////////////////////////////////
bool operator!=(const Path& path, const std::string& str)
{
    return path.m_impl->fsPath != str;
}

} // namespace sf
