// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Packet.hpp"

#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/AssertAndAssume.hpp"
#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"

#include <string>


namespace za
{
////////////////////////////////////////////////////////////
Packet& Packet::append(const void* data, zb::SizeT sizeInBytes)
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(sizeInBytes > 0);

    m_data.emplaceRange(reinterpret_cast<const unsigned char*>(data), sizeInBytes);
    return *this;
}


////////////////////////////////////////////////////////////
zb::SizeT Packet::getReadPosition() const
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
zb::SizeT Packet::getDataSize() const
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
Packet& Packet::readBytes(void* dst, zb::SizeT size)
{
    if (checkSize(size))
    {
        ZB_MEMCPY(dst, &m_data[m_readPos], size);
        m_readPos += size;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(bool& data)
{
    zb::U8 value = 0;
    if (*this >> value)
        data = (value != 0);

    return *this;
}


////////////////////////////////////////////////////////////
// clang-format off
Packet& Packet::operator>>(zb::I8& data)  { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::U8& data)  { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::I16& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::U16& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::I32& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::U32& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::I64& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(zb::U64& data) { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(float& data)     { return readBytes(&data, sizeof(data)); }
Packet& Packet::operator>>(double& data)    { return readBytes(&data, sizeof(data)); }
// clang-format on


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::string& data)
{
    zb::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        data.assign(reinterpret_cast<char*>(&m_data[m_readPos]), length);
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(zb::String& data)
{
    zb::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        data.assign(reinterpret_cast<char*>(&m_data[m_readPos]), length);
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(std::wstring& data)
{
    zb::U32 length = 0;
    *this >> length;

    data.clear();

    // Bound length so `length * sizeof(U32)` cannot wrap on 32-bit SizeT;
    // also reject lengths larger than the remaining bytes in one shot.
    if (length > 0 && length <= remaining() / sizeof(zb::U32))
    {
        // Per-element `checkSize` is unnecessary: the bound above already
        // guarantees `length * 4` bytes are available from `m_readPos`.
        data.reserve(length);
        for (zb::U32 i = 0; i < length; ++i)
        {
            zb::U32 character = 0;
            ZB_MEMCPY(&character, &m_data[m_readPos], sizeof(character));
            m_readPos += sizeof(character);
            data += static_cast<wchar_t>(character);
        }
    }
    else if (length > 0)
    {
        m_isValid = false;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator>>(Utf8String& data)
{
    zb::U32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        data.reserve(length);
        data.append(reinterpret_cast<const char*>(&m_data[m_readPos]), length);
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
// clang-format off
Packet& Packet::operator<<(bool data)      { return *this << static_cast<zb::U8>(data); }
Packet& Packet::operator<<(zb::I8 data)  { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::U8 data)  { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::I16 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::U16 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::I32 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::U32 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::I64 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(zb::U64 data) { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(float data)     { return append(&data, sizeof(data)); }
Packet& Packet::operator<<(double data)    { return append(&data, sizeof(data)); }
// clang-format on


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::string& data)
{
    const auto length = static_cast<zb::U32>(data.size());
    *this << length;

    if (length > 0)
        append(data.data(), length);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const zb::String& data)
{
    const auto length = static_cast<zb::U32>(data.size());
    *this << length;

    if (length > 0)
        append(data.data(), length);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const std::wstring& data)
{
    const auto length = static_cast<zb::U32>(data.size());
    *this << length;

    if (length > 0)
    {
        m_data.reserveMore(length * sizeof(zb::U32));
        for (const wchar_t c : data)
            *this << static_cast<zb::U32>(c);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator<<(const Utf8String& data)
{
    const auto length = static_cast<zb::U32>(data.byteSize());
    *this << length;

    if (length > 0)
        append(data.data(), length);

    return *this;
}


////////////////////////////////////////////////////////////
zb::SizeT Packet::remaining() const
{
    // Invariant: every mutator of `m_readPos` advances only after
    // `checkSize` succeeds, so `m_readPos <= m_data.size()` always holds.
    return m_data.size() - m_readPos;
}


////////////////////////////////////////////////////////////
bool Packet::checkSize(zb::SizeT size)
{
    // `remaining()` is bounded by `m_data.size()`, so `size <= remaining()`
    // cannot wrap and subsumes the previous manual overflow check.
    m_isValid = m_isValid && size <= remaining();
    return m_isValid;
}


////////////////////////////////////////////////////////////
zb::SizeT& Packet::getSendPos()
{
    return m_sendPos;
}


////////////////////////////////////////////////////////////
const void* Packet::onSend(zb::SizeT& size)
{
    size = getDataSize();
    return getData();
}


////////////////////////////////////////////////////////////
void Packet::onReceive(const void* data, zb::SizeT size)
{
    append(data, size);
}

} // namespace za
