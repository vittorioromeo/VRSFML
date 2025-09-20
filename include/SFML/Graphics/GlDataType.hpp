#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Data types supported by OpenGL
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] GlDataType : unsigned int
{
    Byte          = 0u,
    UnsignedByte  = 1u,
    Short         = 2u,
    UnsignedShort = 3u,
    Int           = 4u,
    UnsignedInt   = 5u,
    Float         = 6u,
    Double        = 7u
};

} // namespace sf
