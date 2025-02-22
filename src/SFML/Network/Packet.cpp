#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/SocketImpl.hpp"

#include "SFML/System/String.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"

#include <string>

#include <cwchar>


namespace
{
////////////////////////////////////////////////////////////
template <typename IntegerType, typename... Bytes>
[[nodiscard]] constexpr IntegerType byteSequenceToInteger(Bytes... byte)
{
    static_assert(sizeof(IntegerType) >= sizeof...(Bytes), "IntegerType not large enough to contain bytes");

    IntegerType     integer = 0;
    sf::base::SizeT index   = 0;

    return ((integer |= static_cast<IntegerType>(static_cast<IntegerType>(byte) << 8 * index++)), ...);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
void Packet::append(const void* data, base::SizeT sizeInBytes)
{
    if (data && (sizeInBytes > 0))
        m_data.emplaceRange(reinterpret_cast<const unsigned char*>(data), sizeInBytes);
}


////////////////////////////////////////////////////////////
base::SizeT Packet::getReadPosition() const
{
    return m_readPos;
}


////////////////////////////////////////////////////////////
void Packet::clear()
{
    m_data.clear();
    m_readPos = 0;
    m_isValid = true;
}


////////////////////////////////////////////////////////////
const void* Packet::getData() const
{
    return !m_data.empty() ? m_data.data() : nullptr;
}


////////////////////////////////////////////////////////////
base::SizeT Packet::getDataSize() const
{
    return m_data.size();
}


////////////////////////////////////////////////////////////
bool Packet::endOfPacket() const
{
    return m_readPos >= m_data.size();
}


////////////////////////////////////////////////////////////
Packet::operator bool() const
{
    return m_isValid;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(bool& data)
{
    base::U8 value = 0;
    if (*this >> value)
        data = (value != 0);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::I8& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::U8& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::I16& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        data = static_cast<base::I16>(priv::SocketImpl::getNtohs(static_cast<base::U16>(data)));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::U16& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        data = priv::SocketImpl::getNtohs(data);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::I32& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        data = static_cast<base::I32>(priv::SocketImpl::getNtohl(static_cast<base::U32>(data)));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::U32& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        data = priv::SocketImpl::getNtohl(data);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::I64& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        std::byte bytes[sizeof(data)];
        SFML_BASE_MEMCPY(bytes, &m_data[m_readPos], sizeof(data));

        data = byteSequenceToInteger<base::I64>(bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]);

        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(base::U64& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        std::byte bytes[sizeof(data)]{};
        SFML_BASE_MEMCPY(bytes, &m_data[m_readPos], sizeof(data));

        data = byteSequenceToInteger<base::U64>(bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]);

        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(float& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(double& data)
{
    if (checkSize(sizeof(data)))
    {
        SFML_BASE_MEMCPY(&data, &m_data[m_readPos], sizeof(data));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(char* data)
{
    SFML_BASE_ASSERT(data && "Packet::operator>> Data must not be null");

    // First extract string length
    base::U32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        SFML_BASE_MEMCPY(data, &m_data[m_readPos], length);
        data[length] = '\0';

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::string& data)
{
    // First extract string length
    base::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        data.assign(reinterpret_cast<char*>(&m_data[m_readPos]), length);

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(wchar_t* data)
{
    SFML_BASE_ASSERT(data && "Packet::operator>> Data must not be null");

    // First extract string length
    base::U32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length * sizeof(base::U32)))
    {
        // Then extract characters
        for (base::U32 i = 0; i < length; ++i)
        {
            base::U32 character = 0;
            *this >> character;
            data[i] = static_cast<wchar_t>(character);
        }
        data[length] = L'\0';
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::wstring& data)
{
    // First extract string length
    base::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(base::U32)))
    {
        // Then extract characters
        for (base::U32 i = 0; i < length; ++i)
        {
            base::U32 character = 0;
            *this >> character;
            data += static_cast<wchar_t>(character);
        }
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(String& data)
{
    // First extract the string length
    base::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(base::U32)))
    {
        // Then extract characters
        for (base::U32 i = 0; i < length; ++i)
        {
            base::U32 character = 0;
            *this >> character;
            data += static_cast<char32_t>(character);
        }
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(bool data)
{
    *this << static_cast<base::U8>(data);
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::I8 data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::U8 data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::I16 data)
{
    const auto toWrite = static_cast<base::I16>(priv::SocketImpl::getHtons(static_cast<base::U16>(data)));
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::U16 data)
{
    const base::U16 toWrite = priv::SocketImpl::getHtons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::I32 data)
{
    const auto toWrite = static_cast<base::I32>(priv::SocketImpl::getHtonl(static_cast<base::U32>(data)));
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::U32 data)
{
    const base::U32 toWrite = priv::SocketImpl::getHtonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::I64 data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually

    base::U8 toWrite[]{static_cast<base::U8>((data >> 56) & 0xFF),
                       static_cast<base::U8>((data >> 48) & 0xFF),
                       static_cast<base::U8>((data >> 40) & 0xFF),
                       static_cast<base::U8>((data >> 32) & 0xFF),
                       static_cast<base::U8>((data >> 24) & 0xFF),
                       static_cast<base::U8>((data >> 16) & 0xFF),
                       static_cast<base::U8>((data >> 8) & 0xFF),
                       static_cast<base::U8>((data) & 0xFF)};

    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(base::U64 data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually

    base::U8 toWrite[]{static_cast<base::U8>((data >> 56) & 0xFF),
                       static_cast<base::U8>((data >> 48) & 0xFF),
                       static_cast<base::U8>((data >> 40) & 0xFF),
                       static_cast<base::U8>((data >> 32) & 0xFF),
                       static_cast<base::U8>((data >> 24) & 0xFF),
                       static_cast<base::U8>((data >> 16) & 0xFF),
                       static_cast<base::U8>((data >> 8) & 0xFF),
                       static_cast<base::U8>((data) & 0xFF)};

    append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(float data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(double data)
{
    append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const char* data)
{
    SFML_BASE_ASSERT(data && "Packet::operator<< Data must not be null");

    // First insert string length
    const auto length = static_cast<base::U32>(SFML_BASE_STRLEN(data));
    *this << length;

    // Then insert characters
    append(data, length * sizeof(char));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::string& data)
{
    // First insert string length
    const auto length = static_cast<base::U32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
        append(data.c_str(), length * sizeof(std::string::value_type));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const wchar_t* data)
{
    SFML_BASE_ASSERT(data && "Packet::operator<< Data must not be null");

    // First insert string length
    const auto length = static_cast<base::U32>(std::wcslen(data));
    *this << length;

    // Then insert characters
    for (const wchar_t* c = data; *c != L'\0'; ++c)
        *this << static_cast<base::U32>(*c);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::wstring& data)
{
    // First insert string length
    const auto length = static_cast<base::U32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (const wchar_t c : data)
            *this << static_cast<base::U32>(c);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const String& data)
{
    // First insert the string length
    const auto length = static_cast<base::U32>(data.getSize());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (const unsigned int datum : data)
            *this << datum;
    }

    return *this;
}


////////////////////////////////////////////////////////////
bool Packet::checkSize(base::SizeT size)
{
    m_isValid = m_isValid && (m_readPos + size <= m_data.size());

    return m_isValid;
}


////////////////////////////////////////////////////////////
base::SizeT& Packet::getSendPos()
{
    return m_sendPos;
}


////////////////////////////////////////////////////////////
const void* Packet::onSend(base::SizeT& size)
{
    size = getDataSize();
    return getData();
}


////////////////////////////////////////////////////////////
void Packet::onReceive(const void* data, base::SizeT size)
{
    append(data, size);
}

} // namespace sf
