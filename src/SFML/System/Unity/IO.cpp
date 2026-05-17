// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/IO.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathStreamOp.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp" // IWYU pragma: keep (provides `operator>>` for `base::String` used by `IN_STREAMABLE_TYPES_X`)
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/StringViewStreamOp.hpp" // IWYU pragma: keep (provides `operator<<` for `base::StringView` used by `writeToFile`)
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"
#include "SFML/Base/Vector.hpp"

#include <filesystem> // IWYU pragma: keep (used in macros)
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

#include <cstdio>

// Native fast-path file reads. The C++ iostream layer has overhead that becomes noticeable for the small/medium files
// VRSFML typically loads. We bypass it where we can and fall back to C stdio if no native path is available.
#if defined(SFML_SYSTEM_WINDOWS)
    #include "SFML/System/WindowsHeader.hpp"

    #define SFML_PRIV_IO_NATIVE_BACKEND 1 // Windows
#elif __has_include(<fcntl.h>) && __has_include(<unistd.h>) && __has_include(<sys/stat.h>)
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>

    #include <cerrno>

    #define SFML_PRIV_IO_NATIVE_BACKEND 2 // POSIX
#else
    #define SFML_PRIV_IO_NATIVE_BACKEND 0 // Fallback to C stdio
#endif

// Note: `::sf::base::String` is intentionally absent from the macros below. It is handled by dedicated non-template
// `operator<<` overloads on the output stream classes, which avoids an ambiguity with the free `operator<<` from
// `StringStreamOp.hpp` when downstream code includes both headers.

// clang-format off
#define SFML_BASE_OUT_STREAMABLE_TYPES_X(x) \
    x(bool)                    \
                               \
    x(char)                    \
    x(unsigned char)           \
                               \
    x(short)                   \
    x(unsigned short)          \
                               \
    x(int)                     \
    x(unsigned int)            \
                               \
    x(long)                    \
    x(unsigned long)           \
                               \
    x(long long)               \
    x(unsigned long long)      \
                               \
    x(float)                   \
    x(double)                  \
    x(long double)             \
                               \
    x(short*)                  \
    x(int*)                    \
    x(void*)                   \
                               \
    x(const char*)             \
                               \
    x(::std::string_view)      \
    x(::std::string)           \
    x(::std::filesystem::path) \
                               \
    x(::sf::base::StringView)  \
    x(::sf::Path)
// clang-format on


// clang-format off
#define SFML_BASE_IN_STREAMABLE_TYPES_X(x) \
    x(bool)                    \
                               \
    x(char)                    \
    x(unsigned char)           \
                               \
    x(short)                   \
    x(unsigned short)          \
                               \
    x(int)                     \
    x(unsigned int)            \
                               \
    x(long)                    \
    x(unsigned long)           \
                               \
    x(long long)               \
    x(unsigned long long)      \
                               \
    x(float)                   \
    x(double)                  \
    x(long double)             \
                               \
    x(void*)                   \
                               \
    x(::std::string)           \
    x(::std::filesystem::path) \
                               \
    x(::sf::base::String)
// clang-format on


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr std::ios_base::openmode mapFileOpenMode(const sf::FileOpenMode sfmlEnum)
{
    std::ios_base::openmode stlValue = {};

    const auto mapFlag = [&](const auto sfmlFlag, std::ios_base::openmode stlFlag)
    {
        if ((sfmlEnum & sfmlFlag) != sf::FileOpenMode::none)
            stlValue |= stlFlag;
    };

    mapFlag(sf::FileOpenMode::app, std::ios_base::app);
    mapFlag(sf::FileOpenMode::ate, std::ios_base::ate);
    mapFlag(sf::FileOpenMode::bin, std::ios_base::binary);
    mapFlag(sf::FileOpenMode::in, std::ios_base::in);
    mapFlag(sf::FileOpenMode::out, std::ios_base::out);
    mapFlag(sf::FileOpenMode::trunc, std::ios_base::trunc);

    return stlValue;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr std::ios_base::fmtflags mapFormatFlags(const sf::FormatFlags sfmlEnum)
{
    std::ios_base::fmtflags stlValue = {};

    const auto mapFlag = [&](const auto sfmlFlag, std::ios_base::fmtflags stlFlag)
    {
        if ((sfmlEnum & sfmlFlag) != sf::FormatFlags::none)
            stlValue |= stlFlag;
    };

    mapFlag(sf::FormatFlags::boolalpha, std::ios_base::boolalpha);
    mapFlag(sf::FormatFlags::dec, std::ios_base::dec);
    mapFlag(sf::FormatFlags::fixed, std::ios_base::fixed);
    mapFlag(sf::FormatFlags::hex, std::ios_base::hex);
    mapFlag(sf::FormatFlags::internal, std::ios_base::internal);
    mapFlag(sf::FormatFlags::left, std::ios_base::left);
    mapFlag(sf::FormatFlags::oct, std::ios_base::oct);
    mapFlag(sf::FormatFlags::right, std::ios_base::right);
    mapFlag(sf::FormatFlags::scientific, std::ios_base::scientific);
    mapFlag(sf::FormatFlags::showbase, std::ios_base::showbase);
    mapFlag(sf::FormatFlags::showpoint, std::ios_base::showpoint);
    mapFlag(sf::FormatFlags::showpos, std::ios_base::showpos);
    mapFlag(sf::FormatFlags::skipws, std::ios_base::skipws);
    mapFlag(sf::FormatFlags::unitbuf, std::ios_base::unitbuf);
    mapFlag(sf::FormatFlags::uppercase, std::ios_base::uppercase);

    return stlValue;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr std::ios_base::seekdir mapSeekDir(const sf::SeekDir sfmlEnum)
{
    switch (sfmlEnum)
    {
        case sf::SeekDir::beg:
            return std::ios_base::beg;
        case sf::SeekDir::cur:
            return std::ios_base::cur;
        default:
            SFML_BASE_ASSERT(sfmlEnum == sf::SeekDir::end);
            return std::ios_base::end;
    }
}


////////////////////////////////////////////////////////////
// Convert either a `StringView` or a `Path` into a UTF-8 `std::string` for
// use with C stdio's narrow-char `fopen`. Used by the fallback path.
[[maybe_unused, gnu::always_inline]] inline std::string toUtf8FilenameForStdio(sf::base::StringView v)
{
    return v.toString<std::string>();
}


////////////////////////////////////////////////////////////
[[maybe_unused, gnu::always_inline]] inline std::string toUtf8FilenameForStdio(const sf::Path& p)
{
    return p.to<std::string>();
}


////////////////////////////////////////////////////////////
// Fallback implementation that uses C stdio (`fopen` / `fread`). Used on
// platforms where no native fast path is available. Avoids the iostream
// streambuf/locale machinery while staying ISO C portable.
template <typename Filename, typename T>
bool readFromFileFallback(const Filename& filename, T& target, const bool isAppend)
{
    SFML_BASE_ASSERT(!isAppend || SFML_BASE_IS_SAME(T, sf::base::Vector<char>));

    const auto fail = [&]
    {
        sf::priv::errMsg("Failed to read from file '{}'\n", filename);
        return false;
    };

    // `fopen` requires a NUL-terminated path. Both `StringView` and `Path` are
    // routed through the UTF-8 narrow-char API since C stdio only supports it
    // portably (Windows has `_wfopen` but the fallback never compiles there).
    const auto path = toUtf8FilenameForStdio(filename);

    std::FILE* const file = std::fopen(path.c_str(), "rb");

    if (file == nullptr)
        return fail();

    SFML_BASE_SCOPE_GUARD({ std::fclose(file); });

    // Determine size by seeking to the end. `ftell` on a binary stream is not
    // strictly defined by C, but works on every hosted platform that would
    // ever take this fallback branch.
    if (std::fseek(file, 0, SEEK_END) != 0)
        return fail();

    const long rawSize = std::ftell(file);

    if (rawSize < 0 || std::fseek(file, 0, SEEK_SET) != 0)
        return fail();

    if (rawSize == 0)
    {
        if (!isAppend)
            target.clear();

        return true;
    }

    const auto size = static_cast<sf::base::SizeT>(rawSize);

    sf::base::SizeT got = 0u;

    dispatchReadFileContentsIntoBufferImpl(target,
                                           size,
                                           isAppend,
                                           [&](char* buf, sf::base::SizeT n)
    {
        got = std::fread(buf, 1u, n, file);
        return n;
    });

    if (got != size)
        return fail();

    return true;
}


#if SFML_PRIV_IO_NATIVE_BACKEND == 1

////////////////////////////////////////////////////////////
// Common Windows post-open size/limit check used by the StringView and Path
// openers below. `handle` is closed on failure.
[[nodiscard]] bool nativeStatHandle(::HANDLE handle, sf::base::SizeT& outSize)
{
    ::LARGE_INTEGER fileSize{};
    if (!::GetFileSizeEx(handle, &fileSize) || fileSize.QuadPart < 0)
    {
        ::CloseHandle(handle);
        return false;
    }

    // Guard against truncating a >SizeT file size on 32-bit builds.
    using ULL = unsigned long long;
    if (static_cast<ULL>(fileSize.QuadPart) > static_cast<ULL>(static_cast<sf::base::SizeT>(-1)))
    {
        ::CloseHandle(handle);
        return false;
    }

    outSize = static_cast<sf::base::SizeT>(fileSize.QuadPart);
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline ::HANDLE nativeCreateFileW(const wchar_t* nullTerminatedPath)
{
    return ::CreateFileW(nullTerminatedPath,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         nullptr,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                         nullptr);
}


////////////////////////////////////////////////////////////
// Windows native: `CreateFileW` + `GetFileSizeEx` + `ReadFile`.
// Goes through the wide-char API, which (unlike the iostream path that takes
// `std::string`) interprets `filename` as UTF-8 -- an improvement over the
// fallback for non-ASCII paths.
[[nodiscard]] bool nativeOpenAndStat(sf::base::StringView filename, ::HANDLE& outHandle, sf::base::SizeT& outSize)
{
    const int utf8Len = static_cast<int>(filename.size());

    // `MultiByteToWideChar` rejects zero-length input; treat empty filename as failure here.
    if (utf8Len <= 0)
        return false;

    const int wideLen = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.data(), utf8Len, nullptr, 0);
    if (wideLen <= 0)
        return false;

    // `resize_and_overwrite` (C++23) skips the zero-init pass that `wstring(n, L'\0')` would do.
    // `c_str()` is auto-null-terminated past `size()` regardless.
    std::wstring widePath;
    widePath.resize_and_overwrite(static_cast<sf::base::SizeT>(wideLen),
                                  [&](wchar_t* buf, sf::base::SizeT n) -> sf::base::SizeT
    {
        return static_cast<sf::base::SizeT>(
            ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.data(), utf8Len, buf, static_cast<int>(n)));
    });

    const ::HANDLE handle = nativeCreateFileW(widePath.c_str());
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    if (!nativeStatHandle(handle, outSize))
        return false;

    outHandle = handle;
    return true;
}


////////////////////////////////////////////////////////////
// Path-native overload: `Path::c_str()` already returns `const wchar_t*` on
// Windows, so we can hand it straight to `CreateFileW` and skip the UTF-8
// → UTF-16 round-trip the StringView opener performs.
[[nodiscard]] bool nativeOpenAndStat(const sf::Path& filename, ::HANDLE& outHandle, sf::base::SizeT& outSize)
{
    const ::HANDLE handle = nativeCreateFileW(filename.c_str());
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    if (!nativeStatHandle(handle, outSize))
        return false;

    outHandle = handle;
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool nativeReadFully(::HANDLE handle, char* buffer, sf::base::SizeT size)
{
    while (size > 0u)
    {
        // `ReadFile` takes a DWORD; cap each call at a safe chunk size.
        constexpr sf::base::SizeT maxChunk = static_cast<sf::base::SizeT>(0x7f'ff'ff'ffu);
        const ::DWORD             chunk    = static_cast<::DWORD>(size > maxChunk ? maxChunk : size);

        ::DWORD bytesRead = 0u;
        if (!::ReadFile(handle, buffer, chunk, &bytesRead, nullptr) || bytesRead == 0u)
            return false;

        buffer += bytesRead;
        size -= static_cast<sf::base::SizeT>(bytesRead);
    }
    return true;
}


////////////////////////////////////////////////////////////
inline void nativeClose(::HANDLE handle)
{
    ::CloseHandle(handle);
}

#elif SFML_PRIV_IO_NATIVE_BACKEND == 2

////////////////////////////////////////////////////////////
// Common POSIX post-open size/limit check used by the StringView and Path
// openers below. `fd` is closed on failure.
[[nodiscard]] bool nativeStatFd(int fd, sf::base::SizeT& outSize)
{
    struct ::stat st{};
    if (::fstat(fd, &st) != 0 || st.st_size < 0)
    {
        ::close(fd);
        return false;
    }

    using ULL = unsigned long long;
    if (static_cast<ULL>(st.st_size) > static_cast<ULL>(static_cast<sf::base::SizeT>(-1)))
    {
        ::close(fd);
        return false;
    }

    outSize = static_cast<sf::base::SizeT>(st.st_size);
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline int nativeOpenCStr(const char* nullTerminatedPath)
{
    int fd = -1;

    do
    {
        fd = ::open(nullTerminatedPath, O_RDONLY | O_CLOEXEC);
    } while (fd < 0 && errno == EINTR);

    return fd;
}


////////////////////////////////////////////////////////////
// POSIX native: `open` + `fstat` + `read`.
[[nodiscard]] bool nativeOpenAndStat(sf::base::StringView filename, int& outFd, sf::base::SizeT& outSize)
{
    // `open` requires a NUL-terminated path; `StringView` is not guaranteed to be one.
    const auto path = filename.toString<std::string>();

    const int fd = nativeOpenCStr(path.c_str());
    if (fd < 0)
        return false;

    if (!nativeStatFd(fd, outSize))
        return false;

    outFd = fd;
    return true;
}


////////////////////////////////////////////////////////////
// Path-native overload: on Linux/BSD/Emscripten `Path::c_str()` is already a
// UTF-8 `const char*` (per `SFML/System/Path.hpp`'s value_type macro), so it
// can be handed straight to `::open` with no per-call allocation. On other
// POSIX platforms (macOS/iOS/Android) Path stores `wchar_t`, so we go through
// a single `to<std::string>()` UTF-8 conversion -- then call `::open` directly
// rather than delegating to the StringView opener (which would re-convert).
[[nodiscard]] bool nativeOpenAndStat(const sf::Path& filename, int& outFd, sf::base::SizeT& outSize)
{
    #if defined(SFML_SYSTEM_LINUX_OR_BSD) || defined(SFML_SYSTEM_EMSCRIPTEN)
    const int fd = nativeOpenCStr(filename.c_str());
    #else
    const auto utf8 = filename.to<std::string>();
    const int  fd   = nativeOpenCStr(utf8.c_str());
    #endif

    if (fd < 0)
        return false;

    if (!nativeStatFd(fd, outSize))
        return false;

    outFd = fd;
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool nativeReadFully(int fd, char* buffer, sf::base::SizeT size)
{
    while (size > 0u)
    {
        const ::ssize_t got = ::read(fd, buffer, size);

        if (got < 0)
        {
            if (errno == EINTR)
                continue;

            return false;
        }

        if (got == 0)
            return false; // unexpected EOF: file shrank between fstat and read

        buffer += got;
        size -= static_cast<sf::base::SizeT>(got);
    }

    return true;
}


////////////////////////////////////////////////////////////
inline void nativeClose(int fd)
{
    ::close(fd);
}

#endif


template <typename T>
void dispatchReadFileContentsIntoBufferImpl(T& target, const sf::base::SizeT size, const bool isAppend, auto&& fFillBuffer)
{
    if constexpr (SFML_BASE_IS_SAME(T, std::string))
    {
        target.resize_and_overwrite(size, fFillBuffer);
    }
    else if constexpr (SFML_BASE_IS_SAME(T, sf::base::String))
    {
        target.resizeAndOverwrite(size, fFillBuffer);
    }
    else
    {
        static_assert(SFML_BASE_IS_SAME(T, sf::base::Vector<char>));

        const sf::base::SizeT origin    = isAppend ? target.size() : 0u;
        const sf::base::SizeT finalSize = origin + size;

        target.reserve(finalSize);
        fFillBuffer(target.data() + origin, size);
        target.unsafeSetSize(finalSize);
    }
}

////////////////////////////////////////////////////////////
// `Filename` is either `sf::base::StringView` or `const sf::Path&`; both
// resolve to a matching `nativeOpenAndStat` overload above. The Path-taking
// version skips the UTF-8 → UTF-16 conversion on Windows and skips the
// `to<std::string>()` allocation on Linux/BSD/Emscripten.
// `isAppend` is meaningful only when `T == base::Vector<char>`: when true, the
// file's bytes are appended to `target` (preserving existing content); when
// false, `target` is replaced. For string types it must be `false` and the
// behavior is replace.
template <typename Filename, typename T>
bool readFromFileImpl(const Filename& filename, T& target, const bool isAppend)
{
    SFML_BASE_ASSERT(!isAppend || SFML_BASE_IS_SAME(T, sf::base::Vector<char>));

#if SFML_PRIV_IO_NATIVE_BACKEND == 0 // Fallback
    return readFromFileFallback(filename, target, isAppend);
#else
    #if SFML_PRIV_IO_NATIVE_BACKEND == 1 // Windows
    ::HANDLE handle = INVALID_HANDLE_VALUE;
    #else                                // POSIX
    int handle = -1;
    #endif

    sf::base::SizeT size = 0u;
    if (!nativeOpenAndStat(filename, handle, size))
    {
        sf::priv::errMsg("Failed to read from file '{}'\n", filename);
        return false;
    }

    SFML_BASE_SCOPE_GUARD({ nativeClose(handle); });

    if (size == 0u)
    {
        if (!isAppend)
            target.clear();

        return true;
    }

    // `resize_and_overwrite` (std::string, C++23) and the analogous
    // `resizeAndOverwrite` on `sf::base::String` skip the zero-init pass that
    // a plain `resize` would do.
    bool readOk = false;

    dispatchReadFileContentsIntoBufferImpl(target,
                                           size,
                                           isAppend,
                                           [&](char* buf, sf::base::SizeT n)
    {
        readOk = nativeReadFully(handle, buf, n);
        return n;
    });

    if (!readOk)
    {
        sf::priv::errMsg("Failed to read the full contents of file '{}'\n", filename);
        return false;
    }

    return true;
#endif
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct IOStreamInput::Impl
{
    std::istream stream;

    explicit Impl(std::streambuf* sbuf) : stream(sbuf)
    {
    }
};


////////////////////////////////////////////////////////////
IOStreamInput::IOStreamInput(std::streambuf* sbuf) : m_impl(sbuf)
{
}


////////////////////////////////////////////////////////////
std::streambuf* IOStreamInput::rdbuf()
{
    return m_impl->stream.rdbuf();
}


////////////////////////////////////////////////////////////
void IOStreamInput::rdbuf(std::streambuf* sbuf)
{
    m_impl->stream.rdbuf(sbuf);
}


////////////////////////////////////////////////////////////
template <typename T>
IOStreamInput& IOStreamInput::operator>>(T& value)
{
    m_impl->stream >> value;
    return *this;
}


////////////////////////////////////////////////////////////
#define x(type) template IOStreamInput& IOStreamInput::operator>> <type>(type&); // NOLINT(bugprone-macro-parentheses)
SFML_BASE_IN_STREAMABLE_TYPES_X(x)
#undef x


////////////////////////////////////////////////////////////
void IOStreamInput::ignore(base::SizeT count, char delimiter)
{
    m_impl->stream.ignore(static_cast<std::streamsize>(count), delimiter);
}


////////////////////////////////////////////////////////////
void IOStreamInput::clear()
{
    m_impl->stream.clear();
}


////////////////////////////////////////////////////////////
bool IOStreamInput::isGood() const
{
    return m_impl->stream.good();
}


////////////////////////////////////////////////////////////
bool IOStreamInput::isEOF() const
{
    return m_impl->stream.eof();
}


////////////////////////////////////////////////////////////
IOStreamInput::operator bool() const
{
    return static_cast<bool>(m_impl->stream);
}


////////////////////////////////////////////////////////////
IOStreamInput& cIn()
{
    static IOStreamInput stream(std::cin.rdbuf());
    return stream;
}


////////////////////////////////////////////////////////////
template <typename Stream, typename T>
bool getLine(Stream& stream, T& target)
{
    if constexpr (SFML_BASE_IS_SAME(T, std::string))
    {
        return static_cast<bool>(std::getline(stream, target));
    }
    else
    {
        std::string temp;
        const auto  result = static_cast<bool>(std::getline(stream, temp));

        target = base::String{temp.data(), temp.size()};

        return result;
    }
}


////////////////////////////////////////////////////////////
template <typename T>
bool getLine(IOStreamInput& stream, T& target)
{
    if constexpr (SFML_BASE_IS_SAME(T, std::string))
    {
        return static_cast<bool>(std::getline(stream.m_impl->stream, target));
    }
    else
    {
        std::string temp;
        const auto  result = static_cast<bool>(std::getline(stream.m_impl->stream, temp));

        target = base::String{temp.data(), temp.size()};

        return result;
    }
}


////////////////////////////////////////////////////////////
template bool getLine<std::istringstream, std::string>(std::istringstream&, std::string&);
template bool getLine<std::istream, std::string>(std::istream&, std::string&);
template bool getLine<std::istringstream, base::String>(std::istringstream&, base::String&);
template bool getLine<std::istream, base::String>(std::istream&, base::String&);


////////////////////////////////////////////////////////////
template bool getLine<std::string>(IOStreamInput&, std::string&);
template bool getLine<std::string>(InStringStream&, std::string&);
template bool getLine<base::String>(IOStreamInput&, base::String&);
template bool getLine<base::String>(InStringStream&, base::String&);


////////////////////////////////////////////////////////////
bool writeToFile(base::StringView filename, base::StringView contents)
{
    std::ofstream file(filename.toString<std::string>(), std::ios::binary);

    if (!file)
    {
        priv::errMsg("Failed to write to file '{}'\n", filename);
        return false;
    }

    return static_cast<bool>(file << contents);
}


////////////////////////////////////////////////////////////
bool writeToFile(const Path& filename, base::StringView contents)
{
    std::ofstream file(filename.c_str(), std::ios::binary);

    if (!file)
    {
        priv::errMsg("Failed to write to file '{}'\n", filename);
        return false;
    }

    return static_cast<bool>(file << contents);
}


////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, std::string& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, base::String& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, base::Vector<char>& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool readFromFile(const Path& filename, std::string& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool readFromFile(const Path& filename, base::String& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool readFromFile(const Path& filename, base::Vector<char>& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ false);
}


////////////////////////////////////////////////////////////
bool appendFromFile(base::StringView filename, base::Vector<char>& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ true);
}


////////////////////////////////////////////////////////////
bool appendFromFile(const Path& filename, base::Vector<char>& target)
{
    return readFromFileImpl(filename, target, /* isAppend */ true);
}


////////////////////////////////////////////////////////////
base::Vector<char>& getThreadLocalScratchCharBuffer()
{
    static thread_local base::Vector<char> buffer;
    return buffer;
}


////////////////////////////////////////////////////////////
struct OutFileStream::Impl
{
    std::ofstream ofs;

    Impl() = default;

    Impl(auto&&... args) : ofs(static_cast<decltype(args)>(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
OutFileStream::OutFileStream(const Path& filename, FileOpenMode mode) : m_impl(filename.c_str(), mapFileOpenMode(mode))
{
}


////////////////////////////////////////////////////////////
OutFileStream::OutFileStream()                                    = default;
OutFileStream::~OutFileStream()                                   = default;
OutFileStream::OutFileStream(OutFileStream&&) noexcept            = default;
OutFileStream& OutFileStream::operator=(OutFileStream&&) noexcept = default;


////////////////////////////////////////////////////////////
void OutFileStream::open(const Path& filename, FileOpenMode mode)
{
    m_impl->ofs.open(filename.c_str(), mapFileOpenMode(mode));
}


////////////////////////////////////////////////////////////
void OutFileStream::write(const char* data, base::PtrDiffT size)
{
    m_impl->ofs.write(data, static_cast<std::streamsize>(size));
}


////////////////////////////////////////////////////////////
void OutFileStream::flush()
{
    m_impl->ofs.flush();
}


////////////////////////////////////////////////////////////
void OutFileStream::close()
{
    m_impl->ofs.close();
}


////////////////////////////////////////////////////////////
bool OutFileStream::isOpen() const
{
    return m_impl->ofs.is_open();
}


////////////////////////////////////////////////////////////
bool OutFileStream::isGood() const
{
    return m_impl->ofs.good();
}


////////////////////////////////////////////////////////////
void OutFileStream::seekPos(base::PtrDiffT absolutePos)
{
    m_impl->ofs.seekp(static_cast<std::streamoff>(absolutePos));
}


////////////////////////////////////////////////////////////
base::PtrDiffT OutFileStream::tellPos()
{
    return static_cast<base::PtrDiffT>(m_impl->ofs.tellp());
}


////////////////////////////////////////////////////////////
OutFileStream::operator bool() const
{
    return static_cast<bool>(m_impl->ofs);
}


////////////////////////////////////////////////////////////
template <typename T>
OutFileStream& OutFileStream::operator<<(const T& value)
{
    if constexpr (base::isEnum<T>)
    {
        return *m_impl << static_cast<base::UnderlyingType<T>>(value);
    }
    else if constexpr (SFML_BASE_IS_SAME(T, base::StringView))
    {
        m_impl->ofs.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
    else
    {
        m_impl->ofs << value;
    }

    return *this;
}


////////////////////////////////////////////////////////////
#define x(type) template OutFileStream& OutFileStream::operator<< <type>(type const&);
SFML_BASE_OUT_STREAMABLE_TYPES_X(x)
#undef x


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(const base::String& value)
{
    m_impl->ofs.write(value.data(), static_cast<std::streamsize>(value.size()));
    return *this;
}


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(SetFill fill)
{
    m_impl->ofs << std::setfill(fill.c);
    return *this;
}


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(SetWidth width)
{
    m_impl->ofs << std::setw(width.width);
    return *this;
}


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(Hex)
{
    m_impl->ofs << std::hex;
    return *this;
}


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(std::ios_base& (*func)(std::ios_base&))
{
    m_impl->ofs << func;
    return *this;
}


////////////////////////////////////////////////////////////
OutFileStream& OutFileStream::operator<<(std::ostream& (*func)(std::ostream&))
{
    m_impl->ofs << func;
    return *this;
}


////////////////////////////////////////////////////////////
struct OutStringStream::Impl
{
    std::ostringstream oss;

    Impl() = default;

    Impl(auto&&... args) : oss(static_cast<decltype(args)>(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
OutStringStream::OutStringStream()                                      = default;
OutStringStream::~OutStringStream()                                     = default;
OutStringStream::OutStringStream(OutStringStream&&) noexcept            = default;
OutStringStream& OutStringStream::operator=(OutStringStream&&) noexcept = default;


////////////////////////////////////////////////////////////
OutStringStream::OutStringStream(const char* str) : m_impl(str)
{
}


////////////////////////////////////////////////////////////
std::streambuf* OutStringStream::rdbuf() const
{
    return m_impl->oss.rdbuf();
}


////////////////////////////////////////////////////////////
void OutStringStream::write(const char* data, base::PtrDiffT size)
{
    m_impl->oss.write(data, static_cast<std::streamsize>(size));
}


////////////////////////////////////////////////////////////
void OutStringStream::flush()
{
    m_impl->oss.flush();
}


////////////////////////////////////////////////////////////
void OutStringStream::setStr(base::StringView str)
{
    m_impl->oss.str(str.toString<std::string>());
}


////////////////////////////////////////////////////////////
void OutStringStream::setPrecision(base::PtrDiffT precision)
{
    m_impl->oss.precision(static_cast<std::streamsize>(precision));
}


////////////////////////////////////////////////////////////
void OutStringStream::setFormatFlags(FormatFlags flags)
{
    m_impl->oss.setf(mapFormatFlags(flags));
}


////////////////////////////////////////////////////////////
bool OutStringStream::isGood() const
{
    return m_impl->oss.good();
}


////////////////////////////////////////////////////////////
OutStringStream::operator bool() const
{
    return static_cast<bool>(m_impl->oss);
}


////////////////////////////////////////////////////////////
template <typename T>
T OutStringStream::to() const
{
    return T{m_impl->oss.str()};
}


////////////////////////////////////////////////////////////
template Utf8String   OutStringStream::to() const;
template std::string  OutStringStream::to() const;
template base::String OutStringStream::to() const;


////////////////////////////////////////////////////////////
std::string OutStringStream::getString() const
{
    return m_impl->oss.str();
}


////////////////////////////////////////////////////////////
template <typename T>
OutStringStream& OutStringStream::operator<<(const T& value)
{
    if constexpr (base::isEnum<T>)
    {
        m_impl->oss << static_cast<base::UnderlyingType<T>>(value);
    }
    else if constexpr (SFML_BASE_IS_SAME(T, base::StringView))
    {
        m_impl->oss.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
    else
    {
        m_impl->oss << value;
    }

    return *this;
}


////////////////////////////////////////////////////////////
#define x(type) template OutStringStream& OutStringStream::operator<< <type>(type const&);
SFML_BASE_OUT_STREAMABLE_TYPES_X(x)
#undef x


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(const base::String& value)
{
    m_impl->oss.write(value.data(), static_cast<std::streamsize>(value.size()));
    return *this;
}


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(SetFill fill)
{
    m_impl->oss << std::setfill(fill.c);
    return *this;
}


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(SetWidth width)
{
    m_impl->oss << std::setw(width.width);
    return *this;
}


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(Hex)
{
    m_impl->oss << std::hex;
    return *this;
}


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(std::ios_base& (*func)(std::ios_base&))
{
    m_impl->oss << func;
    return *this;
}


////////////////////////////////////////////////////////////
OutStringStream& OutStringStream::operator<<(std::ostream& (*func)(std::ostream&))
{
    m_impl->oss << func;
    return *this;
}


////////////////////////////////////////////////////////////
struct InFileStream::Impl
{
    std::ifstream ifs;

    Impl() = default;

    Impl(auto&&... args) : ifs(static_cast<decltype(args)>(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
InFileStream::InFileStream()                                   = default;
InFileStream::~InFileStream()                                  = default;
InFileStream::InFileStream(InFileStream&&) noexcept            = default;
InFileStream& InFileStream::operator=(InFileStream&&) noexcept = default;


////////////////////////////////////////////////////////////
InFileStream::InFileStream(const Path& filename, FileOpenMode mode) : m_impl(filename.c_str(), mapFileOpenMode(mode))
{
}


////////////////////////////////////////////////////////////
void InFileStream::open(const Path& filename, FileOpenMode mode)
{
    m_impl->ifs.open(filename.c_str(), mapFileOpenMode(mode));
}


////////////////////////////////////////////////////////////
InFileStream& InFileStream::read(char* data, base::PtrDiffT size)
{
    m_impl->ifs.read(data, static_cast<std::streamsize>(size));
    return *this;
}


////////////////////////////////////////////////////////////
void InFileStream::close()
{
    m_impl->ifs.close();
}


////////////////////////////////////////////////////////////
InFileStream& InFileStream::seekg(base::PtrDiffT absolutePos)
{
    m_impl->ifs.seekg(static_cast<std::streamoff>(absolutePos));
    return *this;
}


////////////////////////////////////////////////////////////
InFileStream& InFileStream::seekg(base::PtrDiffT offset, SeekDir dir)
{
    m_impl->ifs.seekg(static_cast<std::streamoff>(offset), mapSeekDir(dir));
    return *this;
}


////////////////////////////////////////////////////////////
base::PtrDiffT InFileStream::gcount() const
{
    return static_cast<base::PtrDiffT>(m_impl->ifs.gcount());
}


////////////////////////////////////////////////////////////
base::PtrDiffT InFileStream::tellg()
{
    return static_cast<base::PtrDiffT>(m_impl->ifs.tellg());
}


////////////////////////////////////////////////////////////
bool InFileStream::isOpen() const
{
    return m_impl->ifs.is_open();
}


////////////////////////////////////////////////////////////
bool InFileStream::isGood() const
{
    return m_impl->ifs.good();
}


////////////////////////////////////////////////////////////
bool InFileStream::isEOF() const
{
    return m_impl->ifs.eof();
}


////////////////////////////////////////////////////////////
InFileStream::operator bool() const
{
    return static_cast<bool>(m_impl->ifs);
}


////////////////////////////////////////////////////////////
template <typename T>
InFileStream& InFileStream::operator>>(T& value)
{
    if constexpr (base::isEnum<T>)
    {
        m_impl->ifs >> static_cast<base::UnderlyingType<T>&>(value);
    }
    else
    {
        m_impl->ifs >> value;
    }

    return *this;
}


//////////////////////////////////////////////////////////////
#define x(type) template InFileStream& InFileStream::operator>> <type>(type&); // NOLINT(bugprone-macro-parentheses)
SFML_BASE_IN_STREAMABLE_TYPES_X(x)
#undef x


////////////////////////////////////////////////////////////
struct InStringStream::Impl
{
    std::istringstream iss;

    Impl() = default;

    Impl(auto&&... args) : iss(static_cast<decltype(args)>(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
InStringStream::InStringStream()                                     = default;
InStringStream::~InStringStream()                                    = default;
InStringStream::InStringStream(InStringStream&&) noexcept            = default;
InStringStream& InStringStream::operator=(InStringStream&&) noexcept = default;


////////////////////////////////////////////////////////////
InStringStream::InStringStream(const std::string& str, FileOpenMode mode) : m_impl(str, mapFileOpenMode(mode))
{
}


////////////////////////////////////////////////////////////
InStringStream::InStringStream(const base::String& str, FileOpenMode mode) :
    InStringStream{std::string{str.data(), str.size()}, mode}
{
}


////////////////////////////////////////////////////////////
InStringStream& InStringStream::get(char& ch)
{
    m_impl->iss.get(ch);
    return *this;
}


////////////////////////////////////////////////////////////
InStringStream& InStringStream::read(char* data, base::PtrDiffT size)
{
    m_impl->iss.read(data, static_cast<std::streamsize>(size));
    return *this;
}


////////////////////////////////////////////////////////////
void InStringStream::clear()
{
    m_impl->iss.clear();
}


////////////////////////////////////////////////////////////
InStringStream& InStringStream::ignore(base::PtrDiffT count, char delim)
{
    m_impl->iss.ignore(static_cast<std::streamsize>(count), delim);
    return *this;
}


////////////////////////////////////////////////////////////
base::PtrDiffT InStringStream::gcount() const
{
    return static_cast<base::PtrDiffT>(m_impl->iss.gcount());
}


////////////////////////////////////////////////////////////
base::PtrDiffT InStringStream::tellg()
{
    return static_cast<base::PtrDiffT>(m_impl->iss.tellg());
}


////////////////////////////////////////////////////////////
bool InStringStream::isGood() const
{
    return m_impl->iss.good();
}


////////////////////////////////////////////////////////////
bool InStringStream::isEOF() const
{
    return m_impl->iss.eof();
}


////////////////////////////////////////////////////////////
InStringStream::operator bool() const
{
    return static_cast<bool>(m_impl->iss);
}


////////////////////////////////////////////////////////////
template <typename T>
InStringStream& InStringStream::operator>>(T& value)
{
    if constexpr (base::isEnum<T>)
    {
        m_impl->iss >> static_cast<base::UnderlyingType<T>&>(value);
    }
    else
    {
        m_impl->iss >> value;
    }

    return *this;
}


//////////////////////////////////////////////////////////////
InStringStream& InStringStream::operator>>(Hex)
{
    m_impl->iss >> std::hex;
    return *this;
}


//////////////////////////////////////////////////////////////
InStringStream& InStringStream::operator>>(base::String& value)
{
    std::string temp;
    m_impl->iss >> temp;
    value = base::String{temp.data(), temp.size()};
    return *this;
}


//////////////////////////////////////////////////////////////
#define x(type) template InStringStream& InStringStream::operator>> <type>(type&); // NOLINT(bugprone-macro-parentheses)
SFML_BASE_IN_STREAMABLE_TYPES_X(x)
#undef x


////////////////////////////////////////////////////////////
template <typename T>
bool getLine(InStringStream& stream, T& target)
{
    return getLine(stream.m_impl->iss, target);
}

} // namespace sf
