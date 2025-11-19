// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Trait/IsSame.hpp"

#include <filesystem>
#include <ostream>
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

    Impl() = default;

    Impl(const auto& source, std::string&& theBuffer) : fsPath{source}, buffer{SFML_BASE_MOVE(theBuffer)}
    {
    }

    Impl(const base::String& source, std::string&& theBuffer) :
        fsPath{std::string(source.data(), source.size())},
        buffer{SFML_BASE_MOVE(theBuffer)}
    {
    }
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

template Path::Path(const base::String&);
template Path::Path(const std::string&);
template Path::Path(const std::basic_string<wchar_t>&);
template Path::Path(const std::u32string&);
template Path::Path(const std::filesystem::path&);

////////////////////////////////////////////////////////////
template <typename T>
Path::Path(const T* source) : m_impl(source, std::string{})
{
}

template Path::Path(const char*);
template Path::Path(const wchar_t*);
template Path::Path(const char32_t*);

////////////////////////////////////////////////////////////
Path::Path(int, const void* fsPath) : m_impl(*static_cast<const std::filesystem::path*>(fsPath), std::string{})
{
}


////////////////////////////////////////////////////////////
Path::~Path()                          = default;
Path::Path(const Path&)                = default;
Path& Path::operator=(const Path&)     = default;
Path::Path(Path&&) noexcept            = default;
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
bool Path::extensionIs(const base::StringView str) const
{
    const auto nativeExt = m_impl->fsPath.extension().native();

    if (nativeExt.size() != str.size())
        return false;

    for (base::SizeT i = 0u; i < nativeExt.size(); ++i)
        if (std::tolower(static_cast<int>(nativeExt[i])) != std::tolower(static_cast<int>(str[i])))
            return false;

    return true;
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
    if constexpr (SFML_BASE_IS_SAME(T, std::filesystem::path))
        return m_impl->fsPath;
    else if constexpr (SFML_BASE_IS_SAME(T, base::String))
    {
        const auto res = m_impl->fsPath.string();
        return base::String{res.data(), res.size()};
    }
    else if constexpr (SFML_BASE_IS_SAME(T, std::string))
        return m_impl->fsPath.string();
    else if constexpr (SFML_BASE_IS_SAME(T, std::u8string))
        return m_impl->fsPath.u8string();
    else if constexpr (SFML_BASE_IS_SAME(T, std::u32string))
        return m_impl->fsPath.u32string();
    else if constexpr (SFML_BASE_IS_SAME(T, std::wstring))
        return m_impl->fsPath.wstring();
    else
    {
        struct unsupported;
        return unsupported{};
    }
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
template std::filesystem::path Path::to<std::filesystem::path>() const;
template std::string           Path::to<std::string>() const;
template base::String          Path::to<base::String>() const;
template std::u8string         Path::to<std::u8string>() const;


////////////////////////////////////////////////////////////
template bool Path::operator== <char>(const char*) const;
template bool Path::operator== <wchar_t>(const wchar_t*) const;
template bool Path::operator== <std::string>(const std::string&) const;


////////////////////////////////////////////////////////////
template bool Path::operator!= <char>(const char*) const;
template bool Path::operator!= <wchar_t>(const wchar_t*) const;
template bool Path::operator!= <std::string>(const std::string&) const;

} // namespace sf
