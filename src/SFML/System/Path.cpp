#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Path.hpp"

#include <filesystem>
#include <string>


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
    mutable std::string   buffer;
};


////////////////////////////////////////////////////////////
Path Path::tempDirectoryPath()
{
    return Path{0, asVoidPtr(std::filesystem::temp_directory_path())};
}


////////////////////////////////////////////////////////////
Path::Path() = default;


////////////////////////////////////////////////////////////
template <typename T>
Path::Path(const T& source) : m_impl(source, std::string{})
{
}

template Path::Path(const std::string&);
template Path::Path(const std::basic_string<wchar_t>&);


////////////////////////////////////////////////////////////
template <typename T>
Path::Path(const T* source) : m_impl(source, std::string{})
{
}

template Path::Path(const char*);
template Path::Path(const wchar_t*);


////////////////////////////////////////////////////////////
Path::Path(int, const void* fsPath) : m_impl(*static_cast<const std::filesystem::path*>(fsPath), std::string{})
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
Path Path::absolute() const
{
    return Path{0, asVoidPtr(std::filesystem::absolute(m_impl->fsPath))};
}


////////////////////////////////////////////////////////////
const Path::value_type* Path::c_str() const
{
    return m_impl->fsPath.c_str();
}


////////////////////////////////////////////////////////////
const char* Path::toCharPtr() const
{
    m_impl->buffer = to<std::string>();

    return m_impl->buffer.c_str();
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
template <typename T>
T Path::to() const
{
    return m_impl->fsPath.string();
}


////////////////////////////////////////////////////////////
template <typename T>
bool Path::operator==(const T* str) const
{
    return m_impl->fsPath == std::filesystem::path(str);
}


////////////////////////////////////////////////////////////
template <typename T>
bool Path::operator!=(const T* str) const
{
    return m_impl->fsPath != std::filesystem::path(str);
}


////////////////////////////////////////////////////////////
template <typename T>
bool Path::operator==(const T& str) const
{
    return m_impl->fsPath == std::filesystem::path(str);
}


////////////////////////////////////////////////////////////
template <typename T>
bool Path::operator!=(const T& str) const
{
    return m_impl->fsPath != std::filesystem::path(str);
}


////////////////////////////////////////////////////////////
template std::string Path::to<std::string>() const;


////////////////////////////////////////////////////////////
template bool Path::operator== <char>(const char*) const;
template bool Path::operator== <wchar_t>(const wchar_t*) const;
template bool Path::operator== <std::string>(const std::string&) const;


////////////////////////////////////////////////////////////
template bool Path::operator!= <char>(const char*) const;
template bool Path::operator!= <wchar_t>(const wchar_t*) const;
template bool Path::operator!= <std::string>(const std::string&) const;

} // namespace sf
