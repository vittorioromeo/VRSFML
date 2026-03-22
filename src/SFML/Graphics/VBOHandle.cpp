// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct VBOHandle::Impl
{
    GLVertexBufferObject vbo;
};


////////////////////////////////////////////////////////////
VBOHandle::VBOHandle()                                = default;
VBOHandle::~VBOHandle()                               = default;
VBOHandle::VBOHandle(VBOHandle&&) noexcept            = default;
VBOHandle& VBOHandle::operator=(VBOHandle&&) noexcept = default;


////////////////////////////////////////////////////////////
void VBOHandle::bind()
{
    m_impl->vbo.bind();
}

} // namespace sf
