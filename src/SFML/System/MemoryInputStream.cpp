// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/MemoryInputStream.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
MemoryInputStream::MemoryInputStream(const void* data, base::SizeT sizeInBytes) :
    m_data(static_cast<const unsigned char*>(data)),
    m_size(sizeInBytes)
{
    SFML_BASE_ASSERT(m_data != nullptr && "MemoryInputStream must be initialized with non-null data");
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> MemoryInputStream::read(void* data, base::SizeT size)
{
    const base::SizeT count = base::min(size, m_size - m_offset);

    if (count > 0)
    {
        SFML_BASE_MEMCPY(data, m_data + m_offset, static_cast<base::SizeT>(count));
        m_offset += count;
    }

    return base::makeOptional(count);
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> MemoryInputStream::seek(base::SizeT position)
{
    m_offset = position < m_size ? position : m_size;
    return base::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> MemoryInputStream::tell()
{
    return base::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> MemoryInputStream::getSize()
{
    return base::makeOptional(m_size);
}

} // namespace sf
