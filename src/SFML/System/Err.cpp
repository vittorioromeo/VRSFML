////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Err.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <filesystem>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>

#include <cstdio>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct ErrStream::Impl
{
    std::ostream stream;
    std::mutex   mutex;

    explicit Impl(std::streambuf* sbuf) : stream(sbuf)
    {
    }
};


////////////////////////////////////////////////////////////
ErrStream::Guard::Guard(std::ostream& stream, void* mutexPtr) : m_stream(stream), m_mutexPtr(mutexPtr)
{
}


////////////////////////////////////////////////////////////
ErrStream::Guard::~Guard()
{
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
ErrStream::ErrStream(std::streambuf* sbuf) : m_impl(priv::makeUnique<Impl>(sbuf))
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
ErrStream& err()
{
    static ErrStream stream(std::cerr.rdbuf());
    return stream;
}


////////////////////////////////////////////////////////////
template <typename T>
ErrStream::Guard ErrStream::operator<<(const T& value)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`
    m_impl->stream << value;

    return Guard{m_impl->stream, &m_impl->mutex};
}


////////////////////////////////////////////////////////////
ErrStream::Guard ErrStream::operator<<(const char* value)
{
    m_impl->mutex.lock(); // Will be unlocked by `~Guard()`
    m_impl->stream << value;

    return Guard{m_impl->stream, &m_impl->mutex};
}


////////////////////////////////////////////////////////////
template ErrStream::Guard ErrStream::operator<< <const char* const>(const char* const&);
template ErrStream::Guard ErrStream::operator<< <long>(const long&);


////////////////////////////////////////////////////////////
ErrStream::Guard& ErrStream::Guard::operator<<(const char* value)
{
    m_stream << value;
    return *this;
}


////////////////////////////////////////////////////////////
template <typename T>
ErrStream::Guard& ErrStream::Guard::operator<<(const T& value)
{
    m_stream << value;
    return *this;
}


////////////////////////////////////////////////////////////
template ErrStream::Guard& ErrStream::Guard::operator<< <bool>(const bool&);
template ErrStream::Guard& ErrStream::Guard::operator<< <char>(const char&);
template ErrStream::Guard& ErrStream::Guard::operator<< <const char* const>(const char* const&);
template ErrStream::Guard& ErrStream::Guard::operator<< <int>(const int&);
template ErrStream::Guard& ErrStream::Guard::operator<< <long>(const long&);
template ErrStream::Guard& ErrStream::Guard::operator<< <short*>(short* const&);
template ErrStream::Guard& ErrStream::Guard::operator<< <std::filesystem::path>(const std::filesystem::path&);
template ErrStream::Guard& ErrStream::Guard::operator<< <std::string_view>(const std::string_view&);
template ErrStream::Guard& ErrStream::Guard::operator<< <std::string>(const std::string&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned int>(const unsigned int&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned long long>(const unsigned long long&);
template ErrStream::Guard& ErrStream::Guard::operator<< <unsigned short>(const unsigned short&);

} // namespace sf::priv
