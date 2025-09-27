#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/EnumClassBitwiseOps.hpp"
#include "SFML/Base/FwdStdString.hpp" // used
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsEnum.hpp"
#include "SFML/Base/Traits/UnderlyingType.hpp"

#include <iosfwd>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
class StringView;
}

namespace sf
{
class Path;
}


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Type used to flush an output stream.
///
////////////////////////////////////////////////////////////
struct FlushType
{
};

////////////////////////////////////////////////////////////
/// \brief Global used to flush an output stream.
///
/// Substitute for `std::flush`.
///
////////////////////////////////////////////////////////////
inline constexpr FlushType flush;


////////////////////////////////////////////////////////////
/// \brief Type used to print a newline and flush an output stream.
///
////////////////////////////////////////////////////////////
struct EndLType
{
};

////////////////////////////////////////////////////////////
/// \brief Global used to print a newline and flush an output stream.
///
/// Substitute for `std::endl`.
///
////////////////////////////////////////////////////////////
inline constexpr EndLType endL;


////////////////////////////////////////////////////////////
/// \brief Output stream wrapper for `std::ostream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API IOStreamOutput
{
    friend IOStreamOutput& cOut();
    friend IOStreamOutput& cErr();

private:
    struct Impl;
    base::InPlacePImpl<Impl, 512> m_impl; //!< Implementation details

public:
    explicit IOStreamOutput(std::streambuf* sbuf);

    std::streambuf* rdbuf();
    void            rdbuf(std::streambuf* sbuf);

    IOStreamOutput& operator<<(std::ios_base& (*func)(std::ios_base&));
    IOStreamOutput& operator<<(std::ostream& (*func)(std::ostream&));

    IOStreamOutput& operator<<(const char* value);
    IOStreamOutput& operator<<(FlushType);
    IOStreamOutput& operator<<(EndLType);

    template <typename T>
    IOStreamOutput& operator<<(const T& value);

    template <decltype(sizeof(int)) N>
    IOStreamOutput& operator<<(const char (&value)[N])
    {
        return operator<<(static_cast<const char*>(value));
    }

    template <typename T>
    IOStreamOutput& operator<<(const T& value)
        requires base::isEnum<T>
    {
        return operator<<(static_cast<base::UnderlyingType<T>>(value));
    }

    std::ostream& getOStream();

    void flush();
};


////////////////////////////////////////////////////////////
/// \brief Input stream wrapper for `std::istream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API IOStreamInput
{
    friend IOStreamInput& cIn();

    template <typename T>
    friend bool getLine(IOStreamInput&, T&);

private:
    struct Impl;
    base::InPlacePImpl<Impl, 512> m_impl; //!< Implementation details

public:
    explicit IOStreamInput(std::streambuf* sbuf);

    std::streambuf* rdbuf();
    void            rdbuf(std::streambuf* sbuf);

    template <typename T>
    IOStreamInput& operator>>(T& value);

    template <typename T>
    IOStreamInput& operator>>(T& value)
        requires base::isEnum<T>
    {
        return operator>>(static_cast<base::UnderlyingType<T>>(value));
    }

    void ignore(base::SizeT count, char delimiter);
    void clear();
};


////////////////////////////////////////////////////////////
/// \brief Stream wrapping the standard output stream `std::cout`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamOutput& cOut();


////////////////////////////////////////////////////////////
/// \brief Stream wrapping the standard error stream `std::cerr`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamOutput& cErr();


////////////////////////////////////////////////////////////
/// \brief Stream wrapping the standard input stream `std::cin`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamInput& cIn();


////////////////////////////////////////////////////////////
/// \brief Helper function to write a string to a file.
///
////////////////////////////////////////////////////////////
bool writeToFile(base::StringView filename, base::StringView contents);


////////////////////////////////////////////////////////////
/// \brief Helper function to read the contents of a file into a string.
///
////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, std::string& target);


////////////////////////////////////////////////////////////
/// \brief Flags used to specify how a file should be opened.
///
/// Substitute for `std::ios_base::openmode`.
///
////////////////////////////////////////////////////////////
enum class FileOpenMode
{
    none  = 0,
    app   = 1L << 0,
    ate   = 1L << 1,
    bin   = 1L << 2,
    in    = 1L << 3,
    out   = 1L << 4,
    trunc = 1L << 5,
};

SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(FileOpenMode);


////////////////////////////////////////////////////////////
/// \brief Flags used to specify how a stream should be formatted.
///
/// Substitute for `std::ios_base::fmtflags`.
///
////////////////////////////////////////////////////////////
enum class FormatFlags
{
    none             = 0,
    boolalpha        = 1L << 0,
    dec              = 1L << 1,
    fixed            = 1L << 2,
    hex              = 1L << 3,
    internal         = 1L << 4,
    left             = 1L << 5,
    oct              = 1L << 6,
    right            = 1L << 7,
    scientific       = 1L << 8,
    showbase         = 1L << 9,
    showpoint        = 1L << 10,
    showpos          = 1L << 11,
    skipws           = 1L << 12,
    unitbuf          = 1L << 13,
    uppercase        = 1L << 14,
    adjustfield      = left | right | internal,
    basefield        = dec | oct | hex,
    floatfield       = scientific | fixed,
    ios_fmtflags_end = 1L << 16,
};

SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(FormatFlags);


////////////////////////////////////////////////////////////
/// \brief Specifies the direction for seeking in a stream.
///
/// Substitute for `std::ios_base::seekdir`.
///
////////////////////////////////////////////////////////////
enum class SeekDir
{
    beg = 0, //!< Beginning of the stream
    cur = 1, //!< Current position in the stream
    end = 2, //!< End of the stream
};


////////////////////////////////////////////////////////////
/// \brief Stream manipulator to set the fill character for an output stream.
///
/// Substitute for `std::setfill`.
///
////////////////////////////////////////////////////////////
struct SetFill
{
    char c; //!< The fill character
};


////////////////////////////////////////////////////////////
/// \brief Stream manipulator to set the field width for an output stream.
///
/// Substitute for `std::setw`.
///
////////////////////////////////////////////////////////////
struct SetWidth
{
    int width; //!< The field width
};


////////////////////////////////////////////////////////////
/// \brief Stream manipulator to set the precision for an output stream.
///
/// Substitute for `std::setprecision`.
///
////////////////////////////////////////////////////////////
struct SetPrecision
{
    int precision; //!< The precision
};


////////////////////////////////////////////////////////////
/// \brief Stream manipulator to set the output format to hexadecimal.
///
/// Substitute for `std::hex`.
///
////////////////////////////////////////////////////////////
struct Hex
{
};


////////////////////////////////////////////////////////////
/// \brief Provides an interface for writing to files, similar to `std::ofstream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class OutFileStream
{
public:
    explicit OutFileStream();
    explicit OutFileStream(const Path& filename, FileOpenMode mode = FileOpenMode::out);

    ~OutFileStream();

    OutFileStream(const OutFileStream&)            = delete;
    OutFileStream& operator=(const OutFileStream&) = delete;

    OutFileStream(OutFileStream&&) noexcept;
    OutFileStream& operator=(OutFileStream&&) noexcept;

    void open(const Path& filename, FileOpenMode mode);
    void write(const char* data, base::PtrDiffT size);
    void flush();
    void close();

    void seekPos(base::PtrDiffT absolutePos);

    [[nodiscard]] base::PtrDiffT tellPos();

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] bool isGood() const;

    [[nodiscard]] explicit operator bool() const;

    template <typename T>
    OutFileStream& operator<<(const T& value);

    OutFileStream& operator<<(SetFill fill);
    OutFileStream& operator<<(SetWidth width);
    OutFileStream& operator<<(Hex hex);
    OutFileStream& operator<<(std::ios_base& (*func)(std::ios_base&));
    OutFileStream& operator<<(std::ostream& (*func)(std::ostream&));

private:
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl; //!< Implementation details
};


////////////////////////////////////////////////////////////
/// \brief Provides an interface for writing to strings, similar to `std::ostringstream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class OutStringStream
{
public:
    explicit OutStringStream();
    explicit OutStringStream(const char* str);

    ~OutStringStream();

    OutStringStream(const OutStringStream&)            = delete;
    OutStringStream& operator=(const OutStringStream&) = delete;

    OutStringStream(OutStringStream&&) noexcept;
    OutStringStream& operator=(OutStringStream&&) noexcept;

    void write(const char* data, base::PtrDiffT size);
    void flush();

    void setStr(base::StringView str);
    void setPrecision(base::PtrDiffT precision);
    void setFormatFlags(FormatFlags flags);

    template <typename T>
    T to() const;

    [[nodiscard]] std::string getString() const;

    [[nodiscard]] bool isGood() const;

    [[nodiscard]] explicit operator bool() const;

    template <typename T>
    OutStringStream& operator<<(const T& value);

    template <auto N>
    OutStringStream& operator<<(const char (&value)[N])
    {
        return operator<<(static_cast<const char*>(value));
    }

    OutStringStream& operator<<(SetFill fill);
    OutStringStream& operator<<(SetWidth width);
    OutStringStream& operator<<(Hex hex);
    OutStringStream& operator<<(std::ios_base& (*func)(std::ios_base&));
    OutStringStream& operator<<(std::ostream& (*func)(std::ostream&));

private:
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl; //!< Implementation details
};


////////////////////////////////////////////////////////////
/// \brief Provides an interface for reading from files, similar to `std::ifstream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class InFileStream
{
public:
    explicit InFileStream();
    explicit InFileStream(const Path& filename, FileOpenMode mode);

    ~InFileStream();

    InFileStream(const InFileStream&)            = delete;
    InFileStream& operator=(const InFileStream&) = delete;

    InFileStream(InFileStream&&) noexcept;
    InFileStream& operator=(InFileStream&&) noexcept;

    void          open(const Path& filename, FileOpenMode mode);
    InFileStream& read(char* data, base::PtrDiffT size);
    void          close();

    InFileStream& seekg(base::PtrDiffT absolutePos);
    InFileStream& seekg(base::PtrDiffT offset, SeekDir dir);

    [[nodiscard]] base::PtrDiffT gcount() const;
    [[nodiscard]] base::PtrDiffT tellg();

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] bool isGood() const;
    [[nodiscard]] bool isEOF() const;

    [[nodiscard]] explicit operator bool() const;

    template <typename T>
    InFileStream& operator>>(T& value);

private:
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl; //!< Implementation details
};


////////////////////////////////////////////////////////////
/// \brief Provides an interface for reading from strings, similar to `std::istringstream`.
///
/// Uses PImpl to avoid exposing expensive headers in the public API.
///
////////////////////////////////////////////////////////////
class InStringStream
{
    template <typename T>
    friend bool getLine(InStringStream&, T&);

public:
    explicit InStringStream();
    explicit InStringStream(const std::string& str, FileOpenMode mode = FileOpenMode::in);

    ~InStringStream();

    InStringStream(const InStringStream&)            = delete;
    InStringStream& operator=(const InStringStream&) = delete;

    InStringStream(InStringStream&&) noexcept;
    InStringStream& operator=(InStringStream&&) noexcept;

    InStringStream& get(char& ch);
    InStringStream& read(char* data, base::PtrDiffT size);

    void            clear();
    InStringStream& ignore(base::PtrDiffT count = 1, char delim = '\n');

    [[nodiscard]] base::PtrDiffT gcount() const;
    [[nodiscard]] base::PtrDiffT tellg();

    [[nodiscard]] bool isGood() const;
    [[nodiscard]] bool isEOF() const;

    [[nodiscard]] explicit operator bool() const;

    template <typename T>
    InStringStream& operator>>(T& value);

private:
    struct Impl;
    base::InPlacePImpl<Impl, 768> m_impl; //!< Implementation details
};


////////////////////////////////////////////////////////////
/// \brief Reads a line from the input stream into the target variable.
///
/// Substitutes for `std::getline`.
///
////////////////////////////////////////////////////////////
template <typename T>
bool getLine(InStringStream& stream, T& target);


////////////////////////////////////////////////////////////
/// \brief Reads a line from the input stream into the target variable.
///
/// Substitutes for `std::getline`.
///
////////////////////////////////////////////////////////////
template <typename Stream, typename T>
bool getLine(Stream& stream, T& target);


////////////////////////////////////////////////////////////
/// \brief Reads a line from the input stream into the target variable.
///
/// Substitutes for `std::getline`.
///
////////////////////////////////////////////////////////////
template <typename T>
bool getLine(IOStreamInput& stream, T& target);

} // namespace sf


////////////////////////////////////////////////////////////
/// \file SFML/System/IO.hpp
/// \ingroup system
///
/// This header provides classes for file and string input/output operations.
/// It includes wrappers for standard streams like `std::cout`, `std::cin`, and `std::cerr`,
/// as well as specialized classes for reading from and writing to files and strings.
///
/// It also defines various stream manipulators and flags for formatting output.
///
/// The classes are designed to be used in a way that minimizes dependencies on
/// expensive headers, using the PImpl idiom to hide implementation details.
///
/// Functionality and API should be similar to the Standard Library's I/O streams.
///
////////////////////////////////////////////////////////////
