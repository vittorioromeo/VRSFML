// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/MemoryInputStream.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/MinMax.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
MemoryInputStream::MemoryInputStream(const void* data, zb::SizeT sizeInBytes) :
    m_data(static_cast<const unsigned char*>(data)),
    m_size(sizeInBytes)
{
    ZB_ASSERT(m_data != nullptr && "MemoryInputStream must be initialized with non-null data");
}


////////////////////////////////////////////////////////////
zb::Optional<zb::SizeT> MemoryInputStream::read(void* data, zb::SizeT size)
{
    const zb::SizeT count = zb::min(size, m_size - m_offset);

    if (count > 0)
    {
        ZB_MEMCPY(data, m_data + m_offset, static_cast<zb::SizeT>(count));
        m_offset += count;
    }

    return zb::makeOptional(count);
}


////////////////////////////////////////////////////////////
zb::Optional<zb::SizeT> MemoryInputStream::seek(zb::SizeT position)
{
    m_offset = position < m_size ? position : m_size;
    return zb::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
zb::Optional<zb::SizeT> MemoryInputStream::tell()
{
    return zb::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
zb::Optional<zb::SizeT> MemoryInputStream::getSize()
{
    return zb::makeOptional(m_size);
}

} // namespace za
