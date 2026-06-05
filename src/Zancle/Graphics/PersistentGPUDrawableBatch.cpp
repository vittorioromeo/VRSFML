// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/DrawableBatch.hpp"
#include "Zancle/Graphics/IndexType.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/GLUtils/GLBufferObject.hpp"
#include "Zancle/GLUtils/GLPersistentRingBuffer.hpp"
#include "Zancle/GLUtils/GLVAOGroup.hpp"

#include "Zancle/Container/Array.hpp"
#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#include "Zancle/Graphics/DrawableBatchImpl.inl" // IWYU pragma: keep


namespace za::priv
{
namespace
{
////////////////////////////////////////////////////////////
struct FrameState
{
    GLVAOGroup persistentVaoGroup; //!< VAO, VBO, and EBO for this frame state

    mutable GLPersistentRingBuffer<GLVertexBufferObject>  vboRingBuffer; //!< Vertex ring buffer
    mutable GLPersistentRingBuffer<GLElementBufferObject> eboRingBuffer; //!< Index ring buffer

    FrameState() = default;

    ~FrameState()
    {
        eboRingBuffer.destroy(persistentVaoGroup.ebo);
        vboRingBuffer.destroy(persistentVaoGroup.vbo);
    }

    FrameState(const FrameState&)            = delete;
    FrameState& operator=(const FrameState&) = delete;

    FrameState(FrameState&&) noexcept = default;

    FrameState& operator=(FrameState&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        // Must unmap the buffers before moving them
        vboRingBuffer.destroy(persistentVaoGroup.vbo);
        eboRingBuffer.destroy(persistentVaoGroup.ebo);

        persistentVaoGroup = ZA_MOVE(rhs.persistentVaoGroup);
        vboRingBuffer      = ZA_MOVE(rhs.vboRingBuffer);
        eboRingBuffer      = ZA_MOVE(rhs.eboRingBuffer);

        return *this;
    }
};

} // namespace


////////////////////////////////////////////////////////////
struct PersistentGPUStorage::Impl
{
    ////////////////////////////////////////////////////////////
    za::Array<FrameState, 3> frameStates;
    za::SizeT                currentFrameIndex{0u};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] FrameState& current() noexcept
    {
        return frameStates[currentFrameIndex];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] const FrameState& current() const noexcept
    {
        return frameStates[currentFrameIndex];
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void advance() noexcept
    {
        currentFrameIndex = (currentFrameIndex + 1u) % frameStates.size();
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
    // Record the previous frame state's buffer capacities so
    // the next state can be pre-sized, avoiding incremental
    // growth on its first use.
    const auto prevVboCap = impl->current().vboRingBuffer.capacity();
    const auto prevEboCap = impl->current().eboRingBuffer.capacity();

    // Rotate to the next frame state. With N states, the state
    // we rotate into was last used N-1 frames ago, so its fence
    // is almost certainly signaled.
    impl->advance();

    auto& fs = impl->current();

    // Non-blocking reclaim: if the fence is already signaled,
    // the marker is removed here and `drain()` below becomes a
    // pure C++ no-op (no GL calls), avoiding the variable
    // latency of `glClientWaitSync` in the common path.
    fs.vboRingBuffer.reclaim();
    fs.eboRingBuffer.reclaim();
    fs.vboRingBuffer.drain();
    fs.eboRingBuffer.drain();

    // Pre-size if the new state's buffers are smaller than what
    // the previous frame needed. This turns ~20 incremental
    // reallocations into a single up-front allocation.
    if (prevVboCap > fs.vboRingBuffer.capacity())
        fs.vboRingBuffer.reserveCapacity(fs.persistentVaoGroup.vbo, prevVboCap);

    if (prevEboCap > fs.eboRingBuffer.capacity())
        fs.eboRingBuffer.reserveCapacity(fs.persistentVaoGroup.ebo, prevEboCap);

    nVertices = 0u;
    nIndices  = 0u;
}


////////////////////////////////////////////////////////////
Vertex* PersistentGPUStorage::reserveMoreVertices(const za::SizeT count)
{
    auto& fs = impl->current();

    [[maybe_unused]] const auto offset = fs.vboRingBuffer.beginWrite(fs.persistentVaoGroup.vbo, sizeof(Vertex) * count);
    ZA_ASSERT(offset == sizeof(Vertex) * nVertices);

    return static_cast<Vertex*>(fs.vboRingBuffer.data()) + nVertices;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndices(const za::SizeT count)
{
    auto& fs = impl->current();

    [[maybe_unused]] const auto offset = fs.eboRingBuffer.beginWrite(fs.persistentVaoGroup.ebo, sizeof(IndexType) * count);
    ZA_ASSERT(offset == sizeof(IndexType) * nIndices);

    return static_cast<IndexType*>(fs.eboRingBuffer.data()) + nIndices;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveVertexCapacity(const za::SizeT count)
{
    for (auto& fs : impl->frameStates)
        fs.vboRingBuffer.reserveCapacity(fs.persistentVaoGroup.vbo, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveIndexCapacity(const za::SizeT count)
{
    for (auto& fs : impl->frameStates)
        fs.eboRingBuffer.reserveCapacity(fs.persistentVaoGroup.ebo, sizeof(IndexType) * count);
}


////////////////////////////////////////////////////////////
[[nodiscard]] const void* PersistentGPUStorage::getVAOGroup() const
{
    return &impl->current().persistentVaoGroup;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushVertexWritesToGPU(const za::SizeT count, const za::SizeT offset) const
{
    const auto& fs = impl->current();
    fs.vboRingBuffer.flushBytesToGPU(fs.persistentVaoGroup.vbo, sizeof(Vertex) * offset, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushIndexWritesToGPU(const za::SizeT count, const za::SizeT offset) const
{
    const auto& fs = impl->current();
    fs.eboRingBuffer.flushBytesToGPU(fs.persistentVaoGroup.ebo, sizeof(IndexType) * offset, sizeof(IndexType) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::commitPendingDrawSubmission() const
{
    const auto& fs = impl->current();
    fs.vboRingBuffer.commit();
    fs.eboRingBuffer.commit();
}


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace za::priv
