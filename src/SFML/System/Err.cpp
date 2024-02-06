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

#include <iostream>
#include <streambuf>

#include <cstdio>


namespace
{
// This class will be used as the default streambuf of sf::Err,
// it outputs to stderr by default (to keep the default behavior)
class DefaultErrStreamBuf : public std::streambuf
{
public:
    DefaultErrStreamBuf()
    {
        // Allocate the write buffer
        constexpr int size   = 64;
        char*         buffer = new char[size];
        setp(buffer, buffer + size);
    }

    ~DefaultErrStreamBuf() override
    {
        // Synchronize
        sync();

        // Delete the write buffer
        delete[] pbase();
    }

private:
    int overflow(int character) override
    {
        if ((character != EOF) && (pptr() != epptr()))
        {
            // Valid character
            return sputc(static_cast<char>(character));
        }
        else if (character != EOF)
        {
            // Not enough space in the buffer: synchronize output and try again
            sync();
            return overflow(character);
        }
        else
        {
            // Invalid character: synchronize output
            return sync();
        }
    }

    int sync() override
    {
        // Check if there is something into the write buffer
        if (pbase() != pptr())
        {
            // Print the contents of the write buffer into the standard error output
            const auto size = static_cast<std::size_t>(pptr() - pbase());
            std::fwrite(pbase(), 1, size, stderr);

            // Reset the pointer position to the beginning of the write buffer
            setp(pbase(), epptr());
        }

        return 0;
    }
};
} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
ErrStream& err()
{
    static DefaultErrStreamBuf buffer;
    static std::ostream        stream(&buffer);
    static ErrStream           errStream(stream);

    return errStream;
}

ErrStream& errEndl(ErrStream& os)
{
    return os << std::endl;
}

std::ostream& ErrStream::getStream() { return m_os; }

ErrStream& ErrStream::operator<<(const void* value) { m_os << value; return *this; }

ErrStream& ErrStream::operator<<(char value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(short value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(int value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(long value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(long long value) { m_os << value; return *this; }

ErrStream& ErrStream::operator<<(unsigned char value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(unsigned short value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(unsigned int value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(unsigned long value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(unsigned long long value) { m_os << value; return *this; }

ErrStream& ErrStream::operator<<(const char* value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(const std::string& value) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(ErrStream& (*value)(ErrStream&)) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(std::ostream& (*value)(std::ostream&)) { m_os << value; return *this; }
ErrStream& ErrStream::operator<<(std::ios_base& (*value)(std::ios_base&)) { m_os << value; return *this; }

ErrStream& ErrStream::operator<<(const std::string_view& value) { m_os << value; return *this; }

} // namespace sf
