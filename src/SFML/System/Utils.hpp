#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/Base/SizeT.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
// Convert byte sequence into integer
// byteSequenceToInteger<int>(0x12, 0x34, 0x56) == 0x563412
template <typename IntegerType, typename... Bytes>
[[nodiscard]] constexpr IntegerType byteSequenceToInteger(Bytes... byte)
{
    static_assert(sizeof(IntegerType) >= sizeof...(Bytes), "IntegerType not large enough to contain bytes");

    IntegerType integer = 0;
    base::SizeT index   = 0;
    return ((integer |= static_cast<IntegerType>(static_cast<IntegerType>(byte) << 8 * index++)), ...);
}

} // namespace sf::priv
