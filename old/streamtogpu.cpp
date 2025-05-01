


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamToGPU(const unsigned int    bufferId,
                                                             const void* const     data,
                                                             const sf::base::SizeT dataByteCount)
{
#ifdef SFML_OPENGL_ES
    // On OpenGL ES, the "naive" method seems faster, also named buffers are not supported
    glCheck(glBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), data, GL_STREAM_DRAW));
#else
    // For small batches, the "naive" method also seems faster
    if (dataByteCount < sizeof(sf::Vertex) * 64u)
    {
        glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), nullptr, GL_STREAM_DRAW)); // Must orphan first
        glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), data, GL_STREAM_DRAW));
        return;
    }

    // For larger batches, memcpying into a transient mapped buffer seems faster
    glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), nullptr, GL_STREAM_DRAW));

    void* const ptr = glCheck(
        glMapNamedBufferRange(bufferId,
                              0u,
                              static_cast<GLsizeiptr>(dataByteCount),
                              GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT));

    SFML_BASE_MEMCPY(ptr, data, dataByteCount);

    [[maybe_unused]] const auto rc = glCheck(glUnmapNamedBuffer(bufferId));
    SFML_BASE_ASSERT(rc == GL_TRUE);
#endif
}
