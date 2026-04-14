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

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatchImpl.inl" // IWYU pragma: keep


namespace sf::priv
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

        persistentVaoGroup = SFML_BASE_MOVE(rhs.persistentVaoGroup);
        vboRingBuffer      = SFML_BASE_MOVE(rhs.vboRingBuffer);
        eboRingBuffer      = SFML_BASE_MOVE(rhs.eboRingBuffer);

        return *this;
    }
};

} // namespace


////////////////////////////////////////////////////////////
struct PersistentGPUStorage::Impl
{
    ////////////////////////////////////////////////////////////
    base::Array<FrameState, 3> frameStates;
    base::SizeT                currentFrameIndex{0u};


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
Vertex* PersistentGPUStorage::reserveMoreVertices(const base::SizeT count)
{
    auto& fs = impl->current();

    [[maybe_unused]] const auto offset = fs.vboRingBuffer.beginWrite(fs.persistentVaoGroup.vbo, sizeof(Vertex) * count);
    SFML_BASE_ASSERT(offset == sizeof(Vertex) * nVertices);

    return static_cast<Vertex*>(fs.vboRingBuffer.data()) + nVertices;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndices(const base::SizeT count)
{
    auto& fs = impl->current();

    [[maybe_unused]] const auto offset = fs.eboRingBuffer.beginWrite(fs.persistentVaoGroup.ebo, sizeof(IndexType) * count);
    SFML_BASE_ASSERT(offset == sizeof(IndexType) * nIndices);

    return static_cast<IndexType*>(fs.eboRingBuffer.data()) + nIndices;
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveVertexCapacity(const base::SizeT count)
{
    for (auto& fs : impl->frameStates)
        fs.vboRingBuffer.reserveCapacity(fs.persistentVaoGroup.vbo, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::reserveIndexCapacity(const base::SizeT count)
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
void PersistentGPUStorage::flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
{
    const auto& fs = impl->current();
    fs.vboRingBuffer.flushBytesToGPU(fs.persistentVaoGroup.vbo, sizeof(Vertex) * offset, sizeof(Vertex) * count);
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
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

} // namespace sf::priv
