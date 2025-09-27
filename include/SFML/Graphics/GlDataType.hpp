#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
    HalfFloat     = 6u,
    Float         = 7u,
    Double        = 8u
};

} // namespace sf
