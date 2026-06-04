#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Integer type used to index into vertex arrays
///
/// Used by `za::DrawIndexedVerticesSettings` and `za::VertexBuffer`
/// when drawing indexed primitives. The 32-bit width allows
/// addressing very large vertex arrays without overflow.
///
////////////////////////////////////////////////////////////
using IndexType = unsigned int;

} // namespace za
