#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsEnum.hpp"
#include "SFML/Base/Traits/UnderlyingType.hpp"

#include <iosfwd>


////////////////////////////////////////////////////////////
// Forward declaration
////////////////////////////////////////////////////////////
namespace sf::base
{
class StringView;
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

} // namespace sf


////////////////////////////////////////////////////////////
/// TODO P1: docs
///
////////////////////////////////////////////////////////////
