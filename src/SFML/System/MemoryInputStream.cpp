#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/MemoryInputStream.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"

#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
MemoryInputStream::MemoryInputStream(const void* data, std::size_t sizeInBytes) :
m_data(static_cast<const std::byte*>(data)),
m_size(sizeInBytes)
{
    SFML_BASE_ASSERT(m_data != nullptr && "MemoryInputStream must be initialized with non-null data");
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> MemoryInputStream::read(void* data, std::size_t size)
{
    const std::size_t count = base::min(size, m_size - m_offset);

    if (count > 0)
    {
        std::memcpy(data, m_data + m_offset, static_cast<std::size_t>(count));
        m_offset += count;
    }

    return base::makeOptional(count);
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> MemoryInputStream::seek(std::size_t position)
{
    m_offset = position < m_size ? position : m_size;
    return base::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> MemoryInputStream::tell()
{
    return base::makeOptional(m_offset);
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> MemoryInputStream::getSize()
{
    return base::makeOptional(m_size);
}

} // namespace sf
