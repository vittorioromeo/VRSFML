#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
struct FlushType
{
};

inline constexpr FlushType flush;


////////////////////////////////////////////////////////////
struct EndLType
{
};

inline constexpr EndLType endL;


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
/// \brief Stream wrapping the standard output stream (std::cout)
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamOutput& cOut();


////////////////////////////////////////////////////////////
/// \brief Stream wrapping the standard error stream (std::cerr)
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamOutput& cErr();


////////////////////////////////////////////////////////////
/// \brief Stream wrapping the standard input stream (std::cin)
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API IOStreamInput& cIn();


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename Stream, typename T>
bool getLine(Stream& stream, T& target);


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename T>
bool getLine(IOStreamInput& stream, T& target);


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
bool writeToFile(base::StringView filename, base::StringView contents);


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
bool readFromFile(base::StringView filename, std::string& target);


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
enum class FileOpenMode
{
    none      = 0,
    app       = 1L << 0,
    ate       = 1L << 1,
    bin       = 1L << 2,
    in        = 1L << 3,
    out       = 1L << 4,
    trunc     = 1L << 5,
    noreplace = 1L << 6,
};

SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(FileOpenMode);


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
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
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
enum class SeekDir
{
    beg = 0, //!< Beginning of the stream
    cur = 1, //!< Current position in the stream
    end = 2, //!< End of the stream
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct SetFill
{
    char c;
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct SetWidth
{
    int width;
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct Hex
{
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
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
/// \brief TODO P1: docs
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

    [[nodiscard]] const std::string& getString() const;

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
/// \brief TODO P1: docs
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

} // namespace sf


////////////////////////////////////////////////////////////
/// TODO P1: docs
///
////////////////////////////////////////////////////////////
