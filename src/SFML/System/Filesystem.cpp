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
#include <SFML/System/Filesystem.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>


namespace sf
{
struct FilesystemPath::Impl : std::filesystem::path
{
    using std::filesystem::path::path;
};

FilesystemPath::FilesystemPath(std::unique_ptr<Impl>&& impl) : m_impl(std::move(impl))
{
}

FilesystemPath::FilesystemPath() : m_impl(std::make_unique<Impl>())
{
}

FilesystemPath::~FilesystemPath()
{
}

FilesystemPath::FilesystemPath(const std::string& str) : m_impl(std::make_unique<Impl>(str))
{
}

FilesystemPath::FilesystemPath(std::string_view str) : m_impl(std::make_unique<Impl>(str))
{
}

FilesystemPath::FilesystemPath(const char* str) : m_impl(std::make_unique<Impl>(str))
{
}

FilesystemPath& FilesystemPath::operator=(const FilesystemPath& rhs)
{
    m_impl = std::make_unique<Impl>(*rhs.m_impl);
    return *this;
}

const FilesystemPath::value_type* FilesystemPath::c_str() const
{
    return m_impl->c_str();
}

std::string FilesystemPath::string() const
{
    return m_impl->string();
}

FilesystemPath FilesystemPath::extension() const
{
    return std::make_unique<Impl>(m_impl->extension());
}

FilesystemPath FilesystemPath::filename() const
{
    return std::make_unique<Impl>(m_impl->filename());
}

bool FilesystemPath::operator==(const char* str) const
{
    return *m_impl == str;
}

FilesystemPath& FilesystemPath::operator/=(const FilesystemPath& path)
{
    *m_impl /= *path.m_impl;
    return *this;
}

bool FilesystemPath::empty() const
{
    return m_impl->empty();
}

FilesystemPath filesystemTempDirectoryPath()
{
    return std::filesystem::temp_directory_path().string();
}

FilesystemPath filesystemAbsolute(const FilesystemPath& path)
{
    return std::filesystem::absolute(path.string()).string();
}

bool filesystemRemove(const FilesystemPath& path)
{
    return std::filesystem::remove(path.string());
}

FilesystemPath operator/(const FilesystemPath& path, const char* str)
{
    return std::make_unique<FilesystemPath::Impl>(*path.m_impl / str);
}

FilesystemPath operator/(const char* str, const FilesystemPath& path)
{
    return std::make_unique<FilesystemPath::Impl>(str / *path.m_impl);
}

FilesystemPath operator/(const FilesystemPath& pathLhs, const FilesystemPath& pathRhs)
{
    return std::make_unique<FilesystemPath::Impl>(*pathLhs.m_impl / *pathRhs.m_impl);
}

std::ostream& operator<<(std::ostream& os, const FilesystemPath& path)
{
    return os << *path.m_impl;
}

} // namespace sf
