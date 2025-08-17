#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/GLUtils/GLPersistentBuffer.hpp"
#include "SFML/GLUtils/GLVAOGroup.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatchImpl.inl"


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PersistentGPUStorage::Impl
{
    GLVAOGroup persistentVaoGroup; //!< VAO, VBO, and EBO associated with the batch (persistent storage)

    GLPersistentBuffer<GLVertexBufferObject>  vboPersistentBuffer; //!< GPU persistent buffer for vertices
    GLPersistentBuffer<GLElementBufferObject> eboPersistentBuffer; //!< GPU persistent buffer for indices

    Impl() : vboPersistentBuffer(persistentVaoGroup.vbo), eboPersistentBuffer(persistentVaoGroup.ebo)
    {
    }

    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&& rhs) noexcept :
        persistentVaoGroup{SFML_BASE_MOVE(rhs.persistentVaoGroup)},
        vboPersistentBuffer{SFML_BASE_MOVE(rhs.vboPersistentBuffer)},
        eboPersistentBuffer{SFML_BASE_MOVE(rhs.eboPersistentBuffer)}
    {
        // Adjust the persistent buffers to point to the new VAO group (self-referential pointers)
        vboPersistentBuffer.adjustObjPointer(persistentVaoGroup.vbo);
        eboPersistentBuffer.adjustObjPointer(persistentVaoGroup.ebo);
    }

    Impl& operator=(Impl&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        // Must unmap the buffers before moving them
        vboPersistentBuffer.unmapIfNeeded();
        eboPersistentBuffer.unmapIfNeeded();

        persistentVaoGroup  = SFML_BASE_MOVE(rhs.persistentVaoGroup);
        vboPersistentBuffer = SFML_BASE_MOVE(rhs.vboPersistentBuffer);
        eboPersistentBuffer = SFML_BASE_MOVE(rhs.eboPersistentBuffer);

        // Adjust the persistent buffers to point to the new VAO group (self-referential pointers)
        vboPersistentBuffer.adjustObjPointer(persistentVaoGroup.vbo);
        eboPersistentBuffer.adjustObjPointer(persistentVaoGroup.ebo);

        return *this;
    }
};


////////////////////////////////////////////////////////////
PersistentGPUStorage::PersistentGPUStorage() = default;


////////////////////////////////////////////////////////////
PersistentGPUStorage::~PersistentGPUStorage() = default;


////////////////////////////////////////////////////////////
PersistentGPUStorage::PersistentGPUStorage(PersistentGPUStorage&&) noexcept = default;


////////////////////////////////////////////////////////////
PersistentGPUStorage& PersistentGPUStorage::operator=(PersistentGPUStorage&&) noexcept = default;


////////////////////////////////////////////////////////////
Vertex* PersistentGPUStorage::reserveMoreVertices(const base::SizeT count)
{
    impl->vboPersistentBuffer.reserve(sizeof(Vertex) * (nVertices + count));
    return static_cast<Vertex*>(impl->vboPersistentBuffer.data()) + nVertices;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndices(const base::SizeT count)
{
    impl->eboPersistentBuffer.reserve(sizeof(IndexType) * (nIndices + count));
    return static_cast<IndexType*>(impl->eboPersistentBuffer.data()) + nIndices;
}


////////////////////////////////////////////////////////////
[[nodiscard]] const void* PersistentGPUStorage::getVAOGroup() const
{
    return &impl->persistentVaoGroup;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset)
{
    impl->vboPersistentBuffer.flushWritesToGPU(sizeof(Vertex), count, offset);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset)
{
    impl->eboPersistentBuffer.flushWritesToGPU(sizeof(IndexType), count, offset);
}


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv
