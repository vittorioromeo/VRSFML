// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLPersistentBuffer.hpp"

#include "Zancle/GLUtils/GLBufferObject.hpp"


namespace za
{
////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class GLPersistentBuffer<GLVertexBufferObject>;
template class GLPersistentBuffer<GLElementBufferObject>;

} // namespace za
