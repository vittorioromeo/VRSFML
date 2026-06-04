// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLBufferObject.hpp"
#include "Zancle/GLUtils/GLPersistentBuffer.hpp"


namespace za
{
////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class GLPersistentBuffer<GLVertexBufferObject>;
template class GLPersistentBuffer<GLElementBufferObject>;

} // namespace za
