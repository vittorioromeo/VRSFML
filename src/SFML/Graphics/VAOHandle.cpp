// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VAOHandle.hpp"

#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLVAOGroup.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct VAOHandle::Impl
{
    GLVAOGroup              vaoGroup;
    base::Vector<VBOHandle> vbos;
    base::SizeT             nextVboSlot{0u};
    base::SizeT             lastFrameCounter{~base::SizeT{0u}};
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


////////////////////////////////////////////////////////////
void VAOHandle::bindVBO(const base::SizeT index)
{
    auto& vbos = m_impl->vbos;

    // Lazily create VBOs up to the requested index
    while (vbos.size() <= index)
        vbos.emplace(vbos.end());

    vbos[index].bind();
}


////////////////////////////////////////////////////////////
void VAOHandle::bindNextVBO()
{
    bindVBO(m_impl->nextVboSlot++);
}


////////////////////////////////////////////////////////////
void VAOHandle::resetVBOSlotsIfNewFrame(const base::SizeT frameCounter)
{
    if (m_impl->lastFrameCounter != frameCounter)
    {
        m_impl->nextVboSlot      = 0u;
        m_impl->lastFrameCounter = frameCounter;
    }
}


} // namespace sf
