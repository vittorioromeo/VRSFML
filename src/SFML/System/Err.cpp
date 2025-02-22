#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"

#include "SFML/Base/StackTrace.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Traits/IsSame.hpp"

#include <atomic>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct ErrStream::Impl
{
    std::ostream      stream;
    std::mutex        mutex;
    std::atomic<bool> multiLine;

    explicit Impl(std::streambuf* sbuf) : stream(sbuf)
    {
    }
};


////////////////////////////////////////////////////////////
ErrStream::Guard::Guard(std::ostream& stream, void* mutexPtr, bool multiLine) :
m_stream(stream),
m_mutexPtr(mutexPtr),
m_multiLine(multiLine)
{
}


////////////////////////////////////////////////////////////
ErrStream::Guard::~Guard()
{
    if (!m_multiLine)
    {
#ifdef SFML_ENABLE_STACK_TRACES
        m_stream << '\n';
        base::priv::printStackTrace();
#endif

        m_stream << '\n' << std::flush;
    }

    static_cast<std::mutex*>(m_mutexPtr)->unlock();
}


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(std::ostream& (*func)(std::ostream&))
{
    return this->operator<< <decltype(func)>(func);
}


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(std::ios_base& (*func)(std::ios_base&))
{
    return this->operator<< <decltype(func)>(func);
}


////////////////////////////////////////////////////////////
ErrStream::ErrStream(std::streambuf* sbuf) : m_impl(sbuf)
{
}


////////////////////////////////////////////////////////////
ErrStream::Guard ErrStream::operator<<(std::ostream& (*func)(std::ostream&))
{
    return this->operator<< <decltype(func)>(func);
}


////////////////////////////////////////////////////////////
std::streambuf* ErrStream::rdbuf()
{
    const std::unique_lock lockGuard(m_impl->mutex);
    return m_impl->stream.rdbuf();
}


////////////////////////////////////////////////////////////
void ErrStream::rdbuf(std::streambuf* sbuf)
{
    const std::unique_lock lockGuard(m_impl->mutex);
    m_impl->stream.rdbuf(sbuf);
}


////////////////////////////////////////////////////////////
ErrStream& err(bool multiLine)
{
    static ErrStream stream(std::cerr.rdbuf());
    stream.m_impl->multiLine.store(multiLine);
    return stream;
}


////////////////////////////////////////////////////////////
template <typename T>
ErrStream::Guard ErrStream::operator<<(const T& value)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`
    m_impl->stream << "[[SFML ERROR]]: " << value;

    return Guard{m_impl->stream, &m_impl->mutex, m_impl->multiLine.load()};
}


////////////////////////////////////////////////////////////
ErrStream::Guard ErrStream::operator<<(const char* value)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`
    m_impl->stream << "[[SFML ERROR]]: " << value;

    return Guard{m_impl->stream, &m_impl->mutex, m_impl->multiLine.load()};
}


////////////////////////////////////////////////////////////
ErrStream::Guard ErrStream::operator<<(ErrFlushType)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`
    m_impl->stream << std::flush;

    return Guard{m_impl->stream, &m_impl->mutex, m_impl->multiLine.load()};
}


////////////////////////////////////////////////////////////
ErrStream::Guard ErrStream::operator<<(PathDebugFormatter pathDebugFormatter)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`

    m_impl->stream << "    Provided path: " << pathDebugFormatter.path.to<std::string>() << '\n'
                   << "    Absolute path: " << pathDebugFormatter.path.absolute();

    return Guard{m_impl->stream, &m_impl->mutex, m_impl->multiLine.load()};
}


////////////////////////////////////////////////////////////
template ErrStream::Guard ErrStream::operator<< <char>(const char&);
template ErrStream::Guard ErrStream::operator<< <const char* const>(const char* const&);
template ErrStream::Guard ErrStream::operator<< <int>(const int&);
template ErrStream::Guard ErrStream::operator<< <long>(const long&);


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(const char* value)
{
    m_stream << value;
    return *this;
}


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(ErrFlushType)
{
    m_stream << std::flush;
    return *this;
}


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(PathDebugFormatter pathDebugFormatter)
{
    m_stream << "    Provided path: " << pathDebugFormatter.path.to<std::string>() << '\n'
             << "    Absolute path: " << pathDebugFormatter.path.absolute();

    return *this;
}


////////////////////////////////////////////////////////////
template <typename T>
ErrStream::Guard& ErrStream::Guard::operator<<(const T& value)
{
    if constexpr (SFML_BASE_IS_SAME(T, base::StringView))
    {
        m_stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
    else
    {
        m_stream << value;
    }

    return *this;
}


////////////////////////////////////////////////////////////
template ErrStream::Guard& ErrStream::Guard::operator<< <base::StringView>(const base::StringView&);
template ErrStream::Guard& ErrStream::Guard::operator<< <bool>(const bool&);
template ErrStream::Guard& ErrStream::Guard::operator<< <char>(const char&);
template ErrStream::Guard& ErrStream::Guard::operator<< <const char* const>(const char* const&);
template ErrStream::Guard& ErrStream::Guard::operator<< <float>(const float&);
template ErrStream::Guard& ErrStream::Guard::operator<< <int>(const int&);
template ErrStream::Guard& ErrStream::Guard::operator<< <long>(const long&);
template ErrStream::Guard& ErrStream::Guard::operator<< <Path>(const Path&);
template ErrStream::Guard& ErrStream::Guard::operator<< <short*>(short* const&);
template ErrStream::Guard& ErrStream::Guard::operator<< <std::string_view>(const std::string_view&);
template ErrStream::Guard& ErrStream::Guard::operator<< <std::string>(const std::string&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned int>(const unsigned int&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned long long>(const unsigned long long&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned long>(const unsigned long&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned short>(const unsigned short&);

} // namespace sf::priv
