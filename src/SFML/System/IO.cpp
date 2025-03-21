#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"

#include "SFML/Base/StackTrace.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Traits/IsSame.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <istream>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>


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
    return static_cast<bool>(std::getline(stream, target));
}


////////////////////////////////////////////////////////////
template <typename T>
bool getLine(IOStreamInput& stream, T& target)
{
    return static_cast<bool>(std::getline(stream.m_impl->stream, target));
}


////////////////////////////////////////////////////////////
template bool getLine<std::istringstream, std::string>(std::istringstream&, std::string&);
template bool getLine<std::istream, std::string>(std::istream&, std::string&);


////////////////////////////////////////////////////////////
template bool getLine<std::string>(IOStreamInput&, std::string&);


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

} // namespace sf
