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
#include "SFML/Graphics/DrawableBatchImpl.inl" // used


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PersistentGPUStorage::Impl
{
    GLVAOGroup persistentVaoGroup; //!< VAO, VBO, and EBO associated with the batch (persistent storage)

    GLPersistentBuffer<GLVertexBufferObject>  vboPersistentBuffer; //!< GPU persistent buffer for vertices
    GLPersistentBuffer<GLElementBufferObject> eboPersistentBuffer; //!< GPU persistent buffer for indices

    // TODO P0: docs, refactor
    GLsync fence{}; //!< Fence for synchronizing CPU/GPU access

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
PersistentGPUStorage::PersistentGPUStorage()                                           = default;
PersistentGPUStorage::~PersistentGPUStorage()                                          = default;
PersistentGPUStorage::PersistentGPUStorage(PersistentGPUStorage&&) noexcept            = default;
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
void PersistentGPUStorage::flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
{
    impl->vboPersistentBuffer.flushWritesToGPU(sizeof(Vertex), count, offset);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
{
    impl->eboPersistentBuffer.flushWritesToGPU(sizeof(IndexType), count, offset);
}


// TODO P0: docs, refactor
void PersistentGPUStorage::startSync()
{
    if (!impl->fence)
        return;

    const GLenum waitResult = glCheck(glClientWaitSync(impl->fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED));

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        priv::err() << "FATAL ERROR: Error waiting on GPU fence";
        base::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED) [[unlikely]]
    {
        priv::err() << "FATAL ERROR: Fence wait timed out";
        base::abort();
    }

    glCheck(glDeleteSync(impl->fence));
    impl->fence = nullptr;
}


// TODO P0: docs, refactor
void PersistentGPUStorage::endSync()
{
    auto& fenceToCreate = impl->fence;
    SFML_BASE_ASSERT(fenceToCreate == nullptr);

    fenceToCreate = glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    if (fenceToCreate == nullptr) [[unlikely]]
    {
        priv::err() << "FATAL ERROR: Error creating fence sync object";
        base::abort();
    }
}


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv
