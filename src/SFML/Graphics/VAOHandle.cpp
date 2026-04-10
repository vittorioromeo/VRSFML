// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VAOHandle.hpp"

#include "SFML/GLUtils/GLVAOGroup.hpp"


namespace sf
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


} // namespace sf
