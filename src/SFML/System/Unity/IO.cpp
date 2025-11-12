// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/IO.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/StackTrace.hpp"
#include "SFML/Base/String.hpp" // used
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/StringViewStreamOp.hpp"
#include "SFML/Base/Trait/IsSame.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>


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

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct IOStreamOutput::Impl
{
    std::ostream stream;

    explicit Impl(std::streambuf* sbuf) : stream(sbuf)
    {
    }
};


////////////////////////////////////////////////////////////
IOStreamOutput::IOStreamOutput(std::streambuf* sbuf) : m_impl(sbuf)
{
}


////////////////////////////////////////////////////////////
std::streambuf* IOStreamOutput::rdbuf()
{
    return m_impl->stream.rdbuf();
}


////////////////////////////////////////////////////////////
void IOStreamOutput::rdbuf(std::streambuf* sbuf)
{
    m_impl->stream.rdbuf(sbuf);
}


////////////////////////////////////////////////////////////
IOStreamOutput& IOStreamOutput::operator<<(const char* value)
{
    m_impl->stream << value;
    return *this;
}


////////////////////////////////////////////////////////////
IOStreamOutput& IOStreamOutput::operator<<(FlushType)
{
    m_impl->stream << std::flush;
    return *this;
}


////////////////////////////////////////////////////////////
IOStreamOutput& IOStreamOutput::operator<<(EndLType)
{
    m_impl->stream << std::endl;
    return *this;
}


////////////////////////////////////////////////////////////
template <typename T>
IOStreamOutput& IOStreamOutput::operator<<(const T& value)
{
    if constexpr (SFML_BASE_IS_SAME(T, base::StringView))
    {
        m_impl->stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
    else
    {
        m_impl->stream << value;
    }

    return *this;
}


////////////////////////////////////////////////////////////
template IOStreamOutput& IOStreamOutput::operator<< <base::String>(const base::String&);
template IOStreamOutput& IOStreamOutput::operator<< <base::StringView>(const base::StringView&);
template IOStreamOutput& IOStreamOutput::operator<< <bool>(const bool&);
template IOStreamOutput& IOStreamOutput::operator<< <char>(const char&);
template IOStreamOutput& IOStreamOutput::operator<< <const char* const>(const char* const&);
template IOStreamOutput& IOStreamOutput::operator<< <float>(const float&);
template IOStreamOutput& IOStreamOutput::operator<< <int>(const int&);
template IOStreamOutput& IOStreamOutput::operator<< <long>(const long&);
template IOStreamOutput& IOStreamOutput::operator<< <Path>(const Path&);
template IOStreamOutput& IOStreamOutput::operator<< <short*>(short* const&);
template IOStreamOutput& IOStreamOutput::operator<< <std::string_view>(const std::string_view&);
template IOStreamOutput& IOStreamOutput::operator<< <std::string>(const std::string&);
template IOStreamOutput& IOStreamOutput::operator<< <unsigned int>(const unsigned int&);
template IOStreamOutput& IOStreamOutput::operator<< <unsigned long long>(const unsigned long long&);
template IOStreamOutput& IOStreamOutput::operator<< <unsigned long>(const unsigned long&);
template IOStreamOutput& IOStreamOutput::operator<< <unsigned short>(const unsigned short&);


////////////////////////////////////////////////////////////
std::ostream& IOStreamOutput::getOStream()
{
    return m_impl->stream;
}


////////////////////////////////////////////////////////////
void IOStreamOutput::flush()
{
    m_impl->stream.flush();
}


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
template IOStreamInput& IOStreamInput::operator>> <bool>(bool&);
template IOStreamInput& IOStreamInput::operator>> <char>(char&);
template IOStreamInput& IOStreamInput::operator>> <float>(float&);
template IOStreamInput& IOStreamInput::operator>> <int>(int&);
template IOStreamInput& IOStreamInput::operator>> <long>(long&);
template IOStreamInput& IOStreamInput::operator>> <std::string>(std::string&);
template IOStreamInput& IOStreamInput::operator>> <base::String>(base::String&);
template IOStreamInput& IOStreamInput::operator>> <unsigned int>(unsigned int&);
template IOStreamInput& IOStreamInput::operator>> <unsigned long long>(unsigned long long&);
template IOStreamInput& IOStreamInput::operator>> <unsigned long>(unsigned long&);
template IOStreamInput& IOStreamInput::operator>> <unsigned short>(unsigned short&);


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
IOStreamOutput& cOut()
{
    static IOStreamOutput stream(std::cout.rdbuf());
    return stream;
}


////////////////////////////////////////////////////////////
IOStreamOutput& cErr()
{
    static IOStreamOutput stream(std::cerr.rdbuf());
    return stream;
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
        priv::err() << "Failed to write to file '" << filename << "'\n";
        return false;
    }

    return static_cast<bool>(file << contents);
}


////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, std::string& target)
{
    std::ifstream file(filename.toString<std::string>(), std::ios::binary);

    if (!file)
    {
        priv::err() << "Failed to read from file '" << filename << "'\n";
        return false;
    }

    return static_cast<bool>(file >> target);
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
OutFileStream::OutFileStream(const Path& filename, FileOpenMode mode) :
    m_impl(filename.to<std::filesystem::path>(), mapFileOpenMode(mode))
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
    m_impl->ofs.open(filename.to<std::filesystem::path>(), mapFileOpenMode(mode));
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
template OutFileStream& OutFileStream::operator<< <base::String>(const base::String&);
template OutFileStream& OutFileStream::operator<< <base::StringView>(const base::StringView&);
template OutFileStream& OutFileStream::operator<< <bool>(const bool&);
template OutFileStream& OutFileStream::operator<< <char>(const char&);
template OutFileStream& OutFileStream::operator<< <const char* const>(const char* const&);
template OutFileStream& OutFileStream::operator<< <float>(const float&);
template OutFileStream& OutFileStream::operator<< <int>(const int&);
template OutFileStream& OutFileStream::operator<< <long>(const long&);
template OutFileStream& OutFileStream::operator<< <Path>(const Path&);
template OutFileStream& OutFileStream::operator<< <short*>(short* const&);
template OutFileStream& OutFileStream::operator<< <std::string_view>(const std::string_view&);
template OutFileStream& OutFileStream::operator<< <std::string>(const std::string&);
template OutFileStream& OutFileStream::operator<< <unsigned int>(const unsigned int&);
template OutFileStream& OutFileStream::operator<< <unsigned long long>(const unsigned long long&);
template OutFileStream& OutFileStream::operator<< <unsigned long>(const unsigned long&);
template OutFileStream& OutFileStream::operator<< <unsigned short>(const unsigned short&);
template OutFileStream& OutFileStream::operator<< <void*>(void* const&);
template OutFileStream& OutFileStream::operator<< <std::filesystem::path>(const std::filesystem::path&);


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
template UnicodeString OutStringStream::to() const;
template std::string   OutStringStream::to() const;
template base::String  OutStringStream::to() const;


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
template OutStringStream& OutStringStream::operator<< <base::String>(const base::String&);
template OutStringStream& OutStringStream::operator<< <base::StringView>(const base::StringView&);
template OutStringStream& OutStringStream::operator<< <bool>(const bool&);
template OutStringStream& OutStringStream::operator<< <char>(const char&);
template OutStringStream& OutStringStream::operator<< <const char*>(const char* const&);
template OutStringStream& OutStringStream::operator<< <float>(const float&);
template OutStringStream& OutStringStream::operator<< <int>(const int&);
template OutStringStream& OutStringStream::operator<< <long>(const long&);
template OutStringStream& OutStringStream::operator<< <Path>(const Path&);
template OutStringStream& OutStringStream::operator<< <short*>(short* const&);
template OutStringStream& OutStringStream::operator<< <std::string_view>(const std::string_view&);
template OutStringStream& OutStringStream::operator<< <std::string>(const std::string&);
template OutStringStream& OutStringStream::operator<< <unsigned int>(const unsigned int&);
template OutStringStream& OutStringStream::operator<< <unsigned long long>(const unsigned long long&);
template OutStringStream& OutStringStream::operator<< <unsigned long>(const unsigned long&);
template OutStringStream& OutStringStream::operator<< <unsigned short>(const unsigned short&);
template OutStringStream& OutStringStream::operator<< <void*>(void* const&);
template OutStringStream& OutStringStream::operator<< <std::filesystem::path>(const std::filesystem::path&);


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
InFileStream::InFileStream(const Path& filename, FileOpenMode mode) :
    m_impl(filename.to<std::filesystem::path>(), mapFileOpenMode(mode))
{
}


////////////////////////////////////////////////////////////
void InFileStream::open(const Path& filename, FileOpenMode mode)
{
    m_impl->ifs.open(filename.to<std::filesystem::path>(), mapFileOpenMode(mode));
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
        m_impl->ifs >> static_cast<base::UnderlyingType<T>>(value);
    }
    else
    {
        m_impl->ifs >> value;
    }

    return *this;
}


//////////////////////////////////////////////////////////////
template InFileStream& InFileStream::operator>> <bool>(bool&);
template InFileStream& InFileStream::operator>> <char>(char&);
template InFileStream& InFileStream::operator>> <float>(float&);
template InFileStream& InFileStream::operator>> <int>(int&);
template InFileStream& InFileStream::operator>> <long>(long&);
template InFileStream& InFileStream::operator>> <short>(short&);
template InFileStream& InFileStream::operator>> <std::string>(std::string&);
template InFileStream& InFileStream::operator>> <unsigned int>(unsigned int&);
template InFileStream& InFileStream::operator>> <unsigned long long>(unsigned long long&);
template InFileStream& InFileStream::operator>> <unsigned long>(unsigned long&);
template InFileStream& InFileStream::operator>> <unsigned short>(unsigned short&);
template InFileStream& InFileStream::operator>> <void*>(void*&);
template InFileStream& InFileStream::operator>> <std::filesystem::path>(std::filesystem::path&);


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
        m_impl->iss >> static_cast<base::UnderlyingType<T>>(value);
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
template InStringStream& InStringStream::operator>> <bool>(bool&);
template InStringStream& InStringStream::operator>> <char>(char&);
template InStringStream& InStringStream::operator>> <float>(float&);
template InStringStream& InStringStream::operator>> <int>(int&);
template InStringStream& InStringStream::operator>> <long>(long&);
template InStringStream& InStringStream::operator>> <short>(short&);
template InStringStream& InStringStream::operator>> <std::string>(std::string&);
template InStringStream& InStringStream::operator>> <unsigned int>(unsigned int&);
template InStringStream& InStringStream::operator>> <unsigned long long>(unsigned long long&);
template InStringStream& InStringStream::operator>> <unsigned long>(unsigned long&);
template InStringStream& InStringStream::operator>> <unsigned short>(unsigned short&);
template InStringStream& InStringStream::operator>> <void*>(void*&);
template InStringStream& InStringStream::operator>> <std::filesystem::path>(std::filesystem::path&);


////////////////////////////////////////////////////////////
template <typename T>
bool getLine(InStringStream& stream, T& target)
{
    return getLine(stream.m_impl->iss, target);
}

} // namespace sf
