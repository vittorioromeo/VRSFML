// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"
#include "SFML/GLUtils/GLPersistentRingBuffer.hpp"
#include "SFML/GLUtils/GLVAOGroup.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatchImpl.inl" // IWYU pragma: keep


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PersistentGPUStorage::Impl
{
    GLVAOGroup persistentVaoGroup; //!< VAO, VBO, and EBO associated with the batch (persistent storage)

    mutable GLPersistentRingBuffer<GLVertexBufferObject>  vboRingBuffer; //!< Vertex ring buffer
    mutable GLPersistentRingBuffer<GLElementBufferObject> eboRingBuffer; //!< Index ring buffer

    Impl() = default;

    ~Impl()
    {
        eboRingBuffer.destroy(persistentVaoGroup.ebo);
        vboRingBuffer.destroy(persistentVaoGroup.vbo);
    }

    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&& rhs) noexcept = default;

    Impl& operator=(Impl&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        // Must unmap the buffers before moving them
        vboRingBuffer.destroy(persistentVaoGroup.vbo);
        eboRingBuffer.destroy(persistentVaoGroup.ebo);

        persistentVaoGroup = SFML_BASE_MOVE(rhs.persistentVaoGroup);
        vboRingBuffer      = SFML_BASE_MOVE(rhs.vboRingBuffer);
        eboRingBuffer      = SFML_BASE_MOVE(rhs.eboRingBuffer);

        return *this;
    }
};


////////////////////////////////////////////////////////////
PersistentGPUStorage::PersistentGPUStorage()                                           = default;
PersistentGPUStorage::~PersistentGPUStorage()                                          = default;
PersistentGPUStorage::PersistentGPUStorage(PersistentGPUStorage&&) noexcept            = default;
PersistentGPUStorage& PersistentGPUStorage::operator=(PersistentGPUStorage&&) noexcept = default;


////////////////////////////////////////////////////////////
void PersistentGPUStorage::clear()
{
    // `PersistentGPUDrawableBatch` can be refilled from worker threads that do
    // not own a GL context. Drain the previous submission here, on the render
    // thread, so the next `add()` only performs mapped-memory writes and never
    // has to wait on GL fences from a context-less worker.
    impl->vboRingBuffer.drain();
    impl->eboRingBuffer.drain();

    nVertices = 0u;
    nIndices  = 0u;
}


////////////////////////////////////////////////////////////
Vertex* PersistentGPUStorage::reserveMoreVertices(const base::SizeT count)
{
    [[maybe_unused]] const auto offset = impl->vboRingBuffer.beginWrite(impl->persistentVaoGroup.vbo, sizeof(Vertex) * count);
    SFML_BASE_ASSERT(offset == sizeof(Vertex) * nVertices);

    return static_cast<Vertex*>(impl->vboRingBuffer.data()) + nVertices;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndices(const base::SizeT count)
{
    [[maybe_unused]] const auto offset = impl->eboRingBuffer.beginWrite(impl->persistentVaoGroup.ebo,
                                                                        sizeof(IndexType) * count);
    SFML_BASE_ASSERT(offset == sizeof(IndexType) * nIndices);

    return static_cast<IndexType*>(impl->eboRingBuffer.data()) + nIndices;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveVertexCapacity(const base::SizeT count)
{
    // `reserve*Capacity()` mirrors the CPU batch reserve APIs: it may grow the
    // backing storage, but it must not stage a write or advance the ring
    // cursor. Examples call `reserveQuads()` for a future frame before
    // `clear()`, so using `beginWrite()` here would incorrectly consume space
    // in the current submission and break the next append.
    impl->vboRingBuffer.reserveCapacity(impl->persistentVaoGroup.vbo, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveIndexCapacity(const base::SizeT count)
{
    impl->eboRingBuffer.reserveCapacity(impl->persistentVaoGroup.ebo, sizeof(IndexType) * count);
}


////////////////////////////////////////////////////////////
[[nodiscard]] const void* PersistentGPUStorage::getVAOGroup() const
{
    return &impl->persistentVaoGroup;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
{
    impl->vboRingBuffer.flushBytesToGPU(impl->persistentVaoGroup.vbo, sizeof(Vertex) * offset, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
{
    impl->eboRingBuffer.flushBytesToGPU(impl->persistentVaoGroup.ebo, sizeof(IndexType) * offset, sizeof(IndexType) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::commitPendingDrawSubmission() const
{
    impl->vboRingBuffer.commit();
    impl->eboRingBuffer.commit();
}


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv
