// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLVAOGroup.hpp"
#include "Zancle/Graphics/VAOHandle.hpp"


namespace za
{
////////////////////////////////////////////////////////////
struct VAOHandle::Impl
{
    GLVAOGroup vaoGroup;
};


////////////////////////////////////////////////////////////
VAOHandle::VAOHandle()                                = default;
VAOHandle::~VAOHandle()                               = default;
VAOHandle::VAOHandle(VAOHandle&&) noexcept            = default;
VAOHandle& VAOHandle::operator=(VAOHandle&&) noexcept = default;


////////////////////////////////////////////////////////////
const GLVAOGroup& VAOHandle::asVAOGroup() const
{
    return m_impl->vaoGroup;
}


} // namespace za
