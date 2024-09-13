    [[gnu::always_inline]] void objectReallocAndMemcpy(
        GLenum          type,
        auto&           object,
        base::SizeT&    capacity,
        const void*     data,
        sf::base::SizeT byteCount)
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

        SFML_BASE_MEMCPY(ptr, data, byteCount);
        glUnmapBuffer(type);
#endif
    }
