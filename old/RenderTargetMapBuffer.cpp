    [[gnu::always_inline]] void objectReallocAndMemcpy(
        GLenum          type,
        auto&           object,
        zb::SizeT&    capacity,
        const void*     data,
        zb::SizeT byteCount)
    {
        reallocObjectIfNeeded(type, object, capacity, byteCount);
#if 1
        glCheck(glBufferSubData(type, 0u, byteCount, data));
#else
        void* ptr = glMapBufferRange(type,
                                     0u,
                                     byteCount,
                                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
                                         GL_MAP_UNSYNCHRONIZED_BIT);

        ZB_MEMCPY(ptr, data, byteCount);
        glUnmapBuffer(type);
#endif
    }
