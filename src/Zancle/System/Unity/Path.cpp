// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Path.hpp"

#include "Zancle/System/Fmt/FmtPath.hpp"
#include "Zancle/System/PathUtils.hpp"

#include "ZancleBase/Fmt/FmtResult.hpp"
#include "ZancleBase/Fmt/FmtSink.hpp"
#include "ZancleBase/Fmt/FmtSpec.hpp"
#include "ZancleBase/FunctionRef.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StdChrono.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Trait/IsSame.hpp"
#include "ZancleBase/Trait/RemoveCVRef.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#include <cstdlib>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] constexpr char asciiToLower(const char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
struct Path::Impl
{
    std::filesystem::path fsPath;

    Impl() = default;

    template <typename T>
        requires(!zb::isSame<zb::RemoveCVRefIndirect<T>, Impl>)
    explicit Impl(T&& source) : fsPath{ZB_FORWARD(source)}
    {
    }

    explicit Impl(const zb::String& source) : fsPath{std::string_view{source.data(), source.size()}}
    {
    }
};


////////////////////////////////////////////////////////////
zb::Optional<Path> Path::getTempDirectory()
{
    std::error_code ec;
    auto            tmp = std::filesystem::temp_directory_path(ec);

    if (ec)
        return zb::nullOpt;

    return zb::makeOptional(Path{0, &tmp});
}


////////////////////////////////////////////////////////////
zb::Optional<Path> Path::getCurrentDirectory()
{
    std::error_code ec;
    auto            cwd = std::filesystem::current_path(ec);

    if (ec)
        return zb::nullOpt;

    return zb::makeOptional(Path{0, &cwd});
}


////////////////////////////////////////////////////////////
bool Path::setCurrentDirectory(const Path& path)
{
    std::error_code ec;
    std::filesystem::current_path(path.m_impl->fsPath, ec);
    return !ec;
}


////////////////////////////////////////////////////////////
zb::Optional<Path> Path::getHomeDirectory()
{
#ifdef ZA_SYSTEM_WINDOWS
    if (const char* const userProfile = std::getenv("USERPROFILE"))
        return zb::makeOptional(Path{userProfile});
#else
    if (const char* const home = std::getenv("HOME"))
        return zb::makeOptional(Path{home});
#endif

    return zb::nullOpt;
}


////////////////////////////////////////////////////////////
Path::Path() = default;


////////////////////////////////////////////////////////////
template <typename T>
Path::Path(const T& source) : m_impl(source)
{
}

template Path::Path(const zb::String&);
template Path::Path(const std::string&);
template Path::Path(const std::basic_string<wchar_t>&);
template Path::Path(const std::u32string&);
template Path::Path(const std::filesystem::path&);

////////////////////////////////////////////////////////////
template <typename T>
Path::Path(const T* source) : m_impl(source)
{
}

template Path::Path(const char*);
template Path::Path(const wchar_t*);
template Path::Path(const char32_t*);

////////////////////////////////////////////////////////////
Path::Path(int, const void* fsPath) : m_impl(*static_cast<const std::filesystem::path*>(fsPath))
{
}


////////////////////////////////////////////////////////////
Path::~Path()                          = default;
Path::Path(const Path&)                = default;
Path& Path::operator=(const Path&)     = default;
Path::Path(Path&&) noexcept            = default;
Path& Path::operator=(Path&&) noexcept = default;


////////////////////////////////////////////////////////////
Path Path::getFilename() const
{
    const auto fn = m_impl->fsPath.filename();
    return Path{0, &fn};
}


////////////////////////////////////////////////////////////
Path Path::getStem() const
{
    const auto s = m_impl->fsPath.stem();
    return Path{0, &s};
}


////////////////////////////////////////////////////////////
Path Path::getExtension() const
{
    const auto ext = m_impl->fsPath.extension();
    return Path{0, &ext};
}


////////////////////////////////////////////////////////////
zb::Optional<Path> Path::getAbsolute() const
{
    std::error_code ec;
    const auto      abs = std::filesystem::absolute(m_impl->fsPath, ec);

    if (ec)
        return zb::nullOpt;

    return zb::makeOptional(Path{0, &abs});
}


////////////////////////////////////////////////////////////
Path Path::getParent() const
{
    const auto p = m_impl->fsPath.parent_path();
    return Path{0, &p};
}


////////////////////////////////////////////////////////////
const Path::value_type* Path::c_str() const
{
    return m_impl->fsPath.c_str();
}


////////////////////////////////////////////////////////////
bool Path::empty() const
{
    return m_impl->fsPath.empty();
}


////////////////////////////////////////////////////////////
bool Path::exists() const
{
    std::error_code ec;
    return std::filesystem::exists(m_impl->fsPath, ec);
}


////////////////////////////////////////////////////////////
bool Path::isDirectory() const
{
    std::error_code ec;
    return std::filesystem::is_directory(m_impl->fsPath, ec);
}


////////////////////////////////////////////////////////////
bool Path::isRegularFile() const
{
    std::error_code ec;
    return std::filesystem::is_regular_file(m_impl->fsPath, ec);
}


////////////////////////////////////////////////////////////
bool Path::isSymlink() const
{
    std::error_code ec;
    return std::filesystem::is_symlink(m_impl->fsPath, ec);
}


////////////////////////////////////////////////////////////
zb::Optional<zb::U64> Path::getFileSize() const
{
    std::error_code ec;
    const auto      size = std::filesystem::file_size(m_impl->fsPath, ec);

    if (ec)
        return zb::nullOpt;

    return zb::makeOptional(static_cast<zb::U64>(size));
}


////////////////////////////////////////////////////////////
zb::Optional<zb::I64> Path::getLastWriteTimeSecondsSinceEpoch() const
{
    std::error_code ec;
    const auto      ftime = std::filesystem::last_write_time(m_impl->fsPath, ec);

    if (ec)
        return zb::nullOpt;

    const auto sysTime = std::chrono::file_clock::to_sys(ftime);
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(sysTime.time_since_epoch()).count();
    return zb::makeOptional(static_cast<zb::I64>(seconds));
}


////////////////////////////////////////////////////////////
bool Path::extensionIs(const zb::StringView str) const
{
    // Delegate the "what is the extension substring" decision to
    // `std::filesystem::path::extension()` so we always match its
    // semantics for `.`, `..`, leading-dot stems, etc.
    // `.native()` returns a reference, so the only allocation is
    // inside `extension()`'s returned path -- which is SSO-friendly
    // for typical extensions like `.png`.
    const auto  extPath   = m_impl->fsPath.extension();
    const auto& nativeExt = extPath.native();

    if (nativeExt.size() != str.size())
        return false;

    for (zb::SizeT i = 0u; i < nativeExt.size(); ++i)
        if (asciiToLower(static_cast<char>(nativeExt[i])) != asciiToLower(str[i]))
            return false;

    return true;
}


////////////////////////////////////////////////////////////
bool Path::hasParent() const
{
    return m_impl->fsPath.has_parent_path();
}


////////////////////////////////////////////////////////////
bool Path::removeFromDisk() const
{
    std::error_code ec;
    return std::filesystem::remove(m_impl->fsPath, ec) && !ec;
}


////////////////////////////////////////////////////////////
bool Path::copyFileTo(const Path& path) const
{
    std::error_code ec;
    return std::filesystem::copy_file(m_impl->fsPath, path.m_impl->fsPath, ec) && !ec;
}


////////////////////////////////////////////////////////////
bool Path::createLeafDirectory() const
{
    std::error_code ec;
    return std::filesystem::create_directory(m_impl->fsPath, ec) && !ec;
}


////////////////////////////////////////////////////////////
bool Path::createDirectoryTree() const
{
    std::error_code ec;
    const bool      created = std::filesystem::create_directories(m_impl->fsPath, ec);

    // `create_directories` returns false when the path already exists; that's not an error.
    return !ec && (created || std::filesystem::is_directory(m_impl->fsPath, ec));
}


////////////////////////////////////////////////////////////
bool Path::renameTo(const Path& target) const
{
    std::error_code ec;
    std::filesystem::rename(m_impl->fsPath, target.m_impl->fsPath, ec);
    return !ec;
}


////////////////////////////////////////////////////////////
bool Path::forEachEntry(zb::FunctionRef<void(const Path&)> callback) const
{
    std::error_code                     ec;
    std::filesystem::directory_iterator it(m_impl->fsPath, ec);

    if (ec)
        return false;

    const std::filesystem::directory_iterator end;
    for (; it != end; it.increment(ec))
    {
        if (ec)
            return false;

        const auto& entryPath = it->path();
        const Path  entry{0, &entryPath};

        callback(entry);
    }

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
    const auto joined = lhs.m_impl->fsPath / rhs.m_impl->fsPath;
    return Path{0, &joined};
}


////////////////////////////////////////////////////////////
Path operator/(Path&& lhs, const Path& rhs)
{
    lhs /= rhs;
    return ZB_MOVE(lhs);
}


////////////////////////////////////////////////////////////
Path& Path::operator+=(const Path& rhs)
{
    m_impl->fsPath += rhs.m_impl->fsPath;
    return *this;
}


////////////////////////////////////////////////////////////
Path operator+(const Path& lhs, const Path& rhs)
{
    auto result = lhs.m_impl->fsPath;
    result += rhs.m_impl->fsPath;
    return Path{0, &result};
}


////////////////////////////////////////////////////////////
template <typename T>
T Path::to() const
{
    if constexpr (ZB_IS_SAME(T, std::filesystem::path))
        return m_impl->fsPath;
    else if constexpr (ZB_IS_SAME(T, zb::String))
    {
        // `u8string()` is locale-independent; `string()` throws on MinGW/Clang64 when the
        // path contains characters outside the current codepage.
        const auto res = m_impl->fsPath.u8string();
        return zb::String{reinterpret_cast<const char*>(res.data()), res.size()};
    }
    else if constexpr (ZB_IS_SAME(T, std::string))
    {
        const auto res = m_impl->fsPath.u8string();
        return std::string{reinterpret_cast<const char*>(res.data()), res.size()};
    }
    else if constexpr (ZB_IS_SAME(T, std::u8string))
        return m_impl->fsPath.u8string();
    else if constexpr (ZB_IS_SAME(T, std::u32string))
        return m_impl->fsPath.u32string();
    else if constexpr (ZB_IS_SAME(T, std::wstring))
        return m_impl->fsPath.wstring();
    else
        static_assert(false,
                      "za::Path::to<T>(): unsupported target type. Supported: std::filesystem::path, "
                      "zb::String, std::string, std::u8string, std::u32string, std::wstring.");
}


////////////////////////////////////////////////////////////
bool Path::operator==(const Path& rhs) const
{
    return m_impl->fsPath == rhs.m_impl->fsPath;
}


////////////////////////////////////////////////////////////
template <typename T>
bool Path::operator==(const T* str) const
{
    return m_impl->fsPath == std::filesystem::path(str);
}


////////////////////////////////////////////////////////////
template std::filesystem::path Path::to<std::filesystem::path>() const;
template std::string           Path::to<std::string>() const;
template zb::String          Path::to<zb::String>() const;
template std::u8string         Path::to<std::u8string>() const;
template std::u32string        Path::to<std::u32string>() const;
template std::wstring          Path::to<std::wstring>() const;


// `operator<<(std::ostream&, const Path&)` lives in `PathStreamOp.cpp`
// -- see the `<ostream>` comment near the top of this file.


////////////////////////////////////////////////////////////
zb::FmtResult fmtArg(zb::FmtSink& sink, const Path& path, const zb::FmtSpec&)
{
    // Same rationale as the stream-insertion operator above: emit UTF-8 to avoid
    // locale-dependent encoding of `std::filesystem::path` on Windows.
    const auto u8 = path.to<std::string>();
    return sink.append(u8.data(), static_cast<zb::SizeT>(u8.size()));
}

} // namespace za


////////////////////////////////////////////////////////////
namespace za::priv
{
////////////////////////////////////////////////////////////
zb::FmtResult fmtArg(zb::FmtSink& sink, const PathDebugFormatter& dbg, const zb::FmtSpec&)
{
    // Two debug lines: input path + resolved absolute path (or sentinel).
    ZB_FMT_TRY(sink.append("    Provided path: ", 19u));
    ZB_FMT_TRY(fmtArg(sink, dbg.path, zb::FmtSpec{}));
    ZB_FMT_TRY(sink.appendChar('\n'));

    ZB_FMT_TRY(sink.append("    Absolute path: ", 19u));

    if (const auto abs = dbg.path.getAbsolute(); abs.hasValue())
        return fmtArg(sink, *abs, zb::FmtSpec{});

    return sink.append("<unavailable>", 13u);
}

} // namespace za::priv


////////////////////////////////////////////////////////////
namespace za
{


////////////////////////////////////////////////////////////
template bool Path::operator== <char>(const char*) const;
template bool Path::operator== <wchar_t>(const wchar_t*) const;
template bool Path::operator== <char32_t>(const char32_t*) const;

} // namespace za
