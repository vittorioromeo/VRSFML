#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/GLUtils/FenceUtils.hpp"
#include "SFML/GLUtils/GLFenceSync.hpp"
#include "SFML/GLUtils/GLPersistentBuffer.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Persistently-mapped buffer with per-commit fence synchronization
///
/// Wraps `sf::GLPersistentBuffer` with a bump-allocator that tracks
/// *submission epochs* as GL fence syncs, so the CPU can safely reuse
/// regions of a persistently-mapped buffer without racing with in-flight
/// GPU reads.
///
/// # Problem
///
/// A persistent mapping (`GL_MAP_PERSISTENT_BIT` + `GL_MAP_UNSYNCHRONIZED_BIT`)
/// exposes a raw CPU pointer into GPU-visible memory. Writes propagate via
/// explicit `glFlushMappedNamedBufferRange`, but the driver does **not**
/// stall on in-flight GPU work -- the caller is responsible for never
/// overwriting a region a pending draw is still reading.
///
/// # Model
///
/// The class tracks a single *linear write cursor* that bump-allocates
/// forward, plus a FIFO of *commit markers*. Each marker records the
/// cursor position at a `commit()` call, paired with a `glFenceSync` that
/// signals once the GPU has finished executing every command issued
/// before the commit:
///
/// ```
///   buffer:  [ ....in flight....|..uncommitted..|...free...]
///             0                  ^              ^           cap
///                                lastCommit    writeCursor
///
///   markers: [ {offset=120, F1}, {offset=180, F2} ]
/// ```
///
/// States:
///   - `[0, lastCommitCursor)`               : in flight (fenced by markers, GPU may still be reading)
///   - `[lastCommitCursor, writeCursor)`     : staged writes, not yet committed
///   - `[writeCursor, capacity)`             : free tail
///
/// When `reclaim()` finds signaled markers at the front of the FIFO, it
/// removes them. Once every marker is gone AND nothing has been staged
/// since the last commit (`writeCursor == lastCommitCursor`), the buffer
/// is fully idle and both cursors reset to `0`, starting the next
/// allocation from the front of the buffer.
///
/// # Usage modes
///
///   - **Ring-buffer streaming** (`sf::VBOHandle`): call `beginWrite` for
///     each upload, `commit()` once after issuing the draw. A single
///     marker covers all uploads in that cycle. `rollback()` discards any
///     uncommitted writes if the draw path unwinds before submission.
///
///   - **Batch filling with offset-from-0 guarantee**
///     (`PersistentGPUDrawableBatch`): call `drain()` at batch reset to
///     stall until the previous batch's fence signals, resetting both
///     cursors to `0`. Subsequent `beginWrite` calls land at the monotonic
///     offsets the batch expects (`0`, `sizeof(Vertex)*nVertices`, ...).
///
/// # Lifecycle
///
/// ```
///   beginWrite ──►  [lastCommit .. writeCursor)  (uncommitted)
///                          │
///                          ├── commit ──► marker inserted
///                          │                 │
///                          │                 │  fence signals
///                          │                 ▼
///                          │              marker erased (reclaim);
///                          │              cursors reset to 0 when idle
///                          │
///                          └── rollback ──► writeCursor rewinds to lastCommit
/// ```
///
/// # Notes
///
/// - Persistent mapping requires desktop OpenGL 4.4+; methods that touch
///   the underlying `GLPersistentBuffer` abort on OpenGL ES.
/// - Within a commit cycle the allocator is *linear*: it does not wrap
///   around until all outstanding markers have been reclaimed. If the
///   cycle exceeds capacity, the underlying buffer is grown. This keeps
///   caller-visible offsets monotonic, which matters for batches that
///   reserve storage incrementally.
/// - `commit()` must be called *after* the draw it fences has been
///   submitted to the GL driver, so the fence covers the draw.
///
////////////////////////////////////////////////////////////
template <typename TBufferObject>
class [[nodiscard]] GLPersistentRingBuffer
{
private:
    ////////////////////////////////////////////////////////////
    struct Marker // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        base::SizeT       offset; //!< writeCursor position captured at commit time
        priv::GLFenceSync fence;  //!< fence that signals when the GPU has processed up to `offset`
    };


    ////////////////////////////////////////////////////////////
    GLPersistentBuffer<TBufferObject> m_persistentBuffer;
    base::SizeT                       m_writeCursor{0u};      //!< next bump-allocation position
    base::SizeT                       m_lastCommitCursor{0u}; //!< writeCursor at the last `commit()`


    ////////////////////////////////////////////////////////////
    base::InPlaceVector<Marker, 8> m_markers;


    ////////////////////////////////////////////////////////////
    /// \brief Reset the cursors to 0 if the buffer is fully idle
    ///
    /// "Idle" = no markers in flight and nothing uncommitted since the
    /// last commit. Returning to offset 0 lets subsequent `beginWrite`
    /// calls restart from the front of the buffer.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resetCursorsIfIdle()
    {
        if (m_markers.empty() && m_writeCursor == m_lastCommitCursor)
        {
            m_writeCursor      = 0u;
            m_lastCommitCursor = 0u;
        }
    }


    ////////////////////////////////////////////////////////////
    /// \brief Block on the oldest marker's fence and erase it
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void waitAndReclaimOldestMarker()
    {
        SFML_BASE_ASSERT(!m_markers.empty());

        priv::waitOnFence(m_markers.front().fence);
        m_markers.erase(m_markers.begin());

        resetCursorsIfIdle();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Grow the underlying persistent buffer to fit the pending write
    ///
    /// Invoked from `handleOverflow` once all in-flight markers
    /// have been reclaimed and the pending write still does not
    /// fit. Computes a target capacity of `max(byteCount, writeCursor
    /// + byteCount)` and defers to `GLPersistentBuffer::reserve`,
    /// which applies its own geometric growth policy on top.
    ///
    /// If the current commit cycle has any live bytes
    /// (`m_writeCursor > 0`), they are preserved across the remap
    /// and re-flushed: the old mapping is invalidated by `reserve`,
    /// and any prior `flushBytesToGPU` calls targeted that old
    /// mapping. Without the re-flush, previously staged data can
    /// become invisible to subsequent draws from the remapped buffer.
    ///
    /// Marked `[[gnu::cold, gnu::noinline]]` so it stays out of
    /// `beginWrite`'s hot path.
    ///
    /// \param obj       Buffer object; will be move-assigned a fresh
    ///                  instance by `reserve()`.
    /// \param byteCount Size of the pending allocation that triggered
    ///                  the grow.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void growInternalStorage(TBufferObject& obj, const base::SizeT byteCount)
    {
        const auto currentCapacity = m_persistentBuffer.capacity();
        const auto targetCapacity  = currentCapacity == 0u ? byteCount : m_writeCursor + byteCount;
        const auto liveByteCount   = m_writeCursor;

        m_persistentBuffer.reserve(obj, targetCapacity, /* preserve */ liveByteCount > 0u);

        // Growing remaps the buffer. Any bytes copied into the new mapping
        // must be flushed again, otherwise earlier uploads from this commit
        // cycle stop being visible to subsequent draws.
        if (liveByteCount > 0u)
            m_persistentBuffer.flushBytesToGPU(obj, /* byteOffset */ 0u, liveByteCount);
    }


public:
    ////////////////////////////////////////////////////////////
    GLPersistentRingBuffer() = default;

    ////////////////////////////////////////////////////////////
    GLPersistentRingBuffer(const GLPersistentRingBuffer&)            = delete;
    GLPersistentRingBuffer& operator=(const GLPersistentRingBuffer&) = delete;


    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    /// Transfers the persistent-buffer metadata, fence markers, and
    /// cursor state to the new ring buffer, leaving `rhs` empty.
    ///
    /// \warning The associated `TBufferObject` is not owned by this type.
    /// The caller must move that buffer object in lockstep with the ring
    /// buffer so the transferred mapping metadata and fences continue to
    /// refer to the correct GL object.
    ///
    /// \warning The move itself does not perform any cleanup of the
    /// underlying GL mapping. Lifetime management remains the
    /// responsibility of the owning type.
    ///
    ////////////////////////////////////////////////////////////
    GLPersistentRingBuffer(GLPersistentRingBuffer&&) noexcept = default;


    ////////////////////////////////////////////////////////////
    /// \brief Move-assign the persistent-buffer metadata, fence markers,
    /// and cursor state
    ///
    /// Leaves `rhs` empty after the transfer.
    ///
    /// \warning The destination must already have been cleaned up by the
    /// owning type via `destroy(obj)` or an equivalent path before this
    /// assignment. `GLPersistentRingBuffer` cannot safely release the old
    /// mapping on its own because it does not own the associated
    /// `TBufferObject`.
    ///
    /// \warning As with the move constructor, the caller must move the
    /// matching `TBufferObject` in lockstep so the transferred metadata
    /// continues to describe the correct GL buffer object.
    ///
    ////////////////////////////////////////////////////////////
    GLPersistentRingBuffer& operator=(GLPersistentRingBuffer&&) noexcept = default;


    ////////////////////////////////////////////////////////////
    /// \brief Release the fence handles and unmap the persistent buffer
    ///
    /// Must be called before the owning object is destroyed. Safe to
    /// call on a default-constructed instance (no-op) or twice in a row.
    ///
    ////////////////////////////////////////////////////////////
    void destroy(TBufferObject& obj)
    {
        m_markers.clear();

        m_writeCursor      = 0u;
        m_lastCommitCursor = 0u;

        m_persistentBuffer.unmapIfNeeded(obj);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] void* data()
    {
        return m_persistentBuffer.data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const void* data() const
    {
        return m_persistentBuffer.data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] base::SizeT capacity() const
    {
        return m_persistentBuffer.capacity();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Flush a mapped byte range so the GPU can see the writes
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushBytesToGPU(const TBufferObject& obj,
                                                const base::SizeT    byteOffset,
                                                const base::SizeT    byteCount) const
    {
        m_persistentBuffer.flushBytesToGPU(obj, byteOffset, byteCount);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Ensure the underlying storage can hold `byteCount` bytes without advancing the write cursor
    ///
    /// Used by callers that want to preallocate capacity ahead of time
    /// without staging any new writes yet. If a grow remaps the buffer,
    /// any currently live bytes are copied into the new mapping and must be
    /// re-flushed so previously staged data stays visible to the GPU.
    ///
    ////////////////////////////////////////////////////////////
    void reserveCapacity(TBufferObject& obj, const base::SizeT byteCount)
    {
        reclaim();

        if (m_persistentBuffer.capacity() >= byteCount)
            return;

        const auto liveByteCount = m_writeCursor;
        m_persistentBuffer.reserve(obj, byteCount, /* preserve */ liveByteCount > 0u);

        if (liveByteCount > 0u)
            m_persistentBuffer.flushBytesToGPU(obj, /* byteOffset */ 0u, liveByteCount);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Bump-allocate `byteCount` bytes and return the write offset
    ///
    /// If the write does not fit at the current cursor, waits for the
    /// oldest marker's fence and retries. If no markers remain and the
    /// write still does not fit, the underlying buffer is grown.
    ///
    /// The returned offset is valid as a direct `data() + offset` write
    /// target. The caller is responsible for calling `flushBytesToGPU`
    /// and `commit()` after the writes are done.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] base::SizeT beginWrite(TBufferObject& obj, const base::SizeT byteCount)
    {
        if (m_writeCursor + byteCount > m_persistentBuffer.capacity()) [[unlikely]]
            handleOverflow(obj, byteCount);

        const auto result = m_writeCursor;
        m_writeCursor += byteCount;
        return result;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Slow path for `beginWrite` when the request exceeds capacity
    ///
    /// Invoked from `beginWrite` when `m_writeCursor + byteCount`
    /// exceeds `capacity()`. Applies three strategies in sequence,
    /// stopping as soon as the pending write fits:
    ///
    /// 1. `reclaim()` -- non-blocking poll that frees any signaled
    ///    markers and, if the buffer is fully idle, resets both
    ///    cursors to `0`. The cheapest path out: the GPU has already
    ///    caught up and the freed region is reusable.
    /// 2. If the write still doesn't fit, block on the oldest pending
    ///    marker's fence and remove it, repeating until either the
    ///    write fits or no markers remain.
    /// 3. If all markers have been reclaimed and the write *still*
    ///    doesn't fit, grow the underlying persistent buffer.
    ///
    /// Split out from `beginWrite` and marked `[[gnu::cold, gnu::noinline]]`
    /// so the common-case hot path (write fits without any marker
    /// traversal) stays a tight `compare + bump`. Keeping the cold
    /// code out-of-line shrinks the hot path's instruction footprint
    /// and improves inlining at the call sites.
    ///
    /// \param obj       Buffer object used by the grow path; unused
    ///                  if the first two strategies succeed.
    /// \param byteCount Size of the pending allocation the caller is
    ///                  about to bump-allocate.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void handleOverflow(TBufferObject& obj, const base::SizeT byteCount)
    {
        reclaim(); // free signaled markers

        while (m_writeCursor + byteCount > m_persistentBuffer.capacity() && !m_markers.empty())
            waitAndReclaimOldestMarker();

        if (m_writeCursor + byteCount > m_persistentBuffer.capacity())
            growInternalStorage(obj, byteCount);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Insert a fence covering every write since the last commit
    ///
    /// Must be called *after* the draw call that consumes the writes has
    /// been submitted, so the fence guards the draw. A no-op if nothing
    /// has been written since the previous commit.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commit()
    {
        if (m_writeCursor == m_lastCommitCursor)
            return;

        m_markers.emplaceBack(m_writeCursor, priv::makeFence());
        m_lastCommitCursor = m_writeCursor;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Rewind the write cursor to the last committed position
    ///
    /// Discards any uncommitted writes. Used by `InstanceAttributeBinder`
    /// when the draw path unwinds before `commit()`.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void rollback() noexcept
    {
        m_writeCursor = m_lastCommitCursor;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Opportunistically reclaim markers whose fences have signaled
    ///
    /// Polls the front of the FIFO with `priv::tryWaitOnFence` (non-blocking).
    /// When the buffer becomes fully idle, the cursors reset to `0`.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reclaim()
    {
        while (!m_markers.empty() && priv::tryWaitOnFence(m_markers.front().fence))
            m_markers.erase(m_markers.begin());

        resetCursorsIfIdle();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Block until every in-flight marker has signaled, then reset
    ///
    /// On return, the marker FIFO is empty and both cursors are at `0`,
    /// regardless of whether uncommitted writes existed beforehand. Any
    /// uncommitted writes are intentionally discarded -- callers that
    /// want to keep them must `commit()` first.
    ///
    /// Use this when the caller requires the next `beginWrite` to start
    /// at offset `0` (e.g. `PersistentGPUDrawableBatch` after `clear()`,
    /// or the auto-batch path at frame start, where the enclosing
    /// framework already synchronized the GPU and the ring's per-commit
    /// fences were never created).
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void drain()
    {
        while (!m_markers.empty())
            waitAndReclaimOldestMarker();

        // Uncommitted writes: no marker covers them, but the caller is
        // explicitly asking for a fresh start. Force the reset.
        m_writeCursor      = 0u;
        m_lastCommitCursor = 0u;
    }
};

} // namespace sf
