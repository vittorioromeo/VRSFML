

////////////////////////////////////////////////////////////
MappedDrawableBatch::MappedDrawableBatch(RenderTarget& renderTarget) : m_renderTarget(renderTarget)
{
}


////////////////////////////////////////////////////////////
void* MappedDrawableBatch::mapBuffer(unsigned int type, base::SizeT allocatedBytes) const
{
    return glCheckExpr(
        glMapBufferRange(type, 0u, allocatedBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
    // return glCheckExpr(glMapBufferRange(type, 0u, allocatedBytes, GL_MAP_WRITE_BIT));
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::unmapBuffer(unsigned int type) const
{
    [[maybe_unused]] const bool rc = glCheckExpr(glUnmapBuffer(type));
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::reallocAndRemapBufferIfNeeded(unsigned int type,
                                                        void*&       bufferPtr,
                                                        base::SizeT& allocatedBytes,
                                                        base::SizeT  targetBytes)
{
    if (allocatedBytes >= targetBytes) [[likely]]
    {
        if (bufferPtr == nullptr)
            bufferPtr = mapBuffer(type, allocatedBytes);

        return;
    }

    allocatedBytes = m_renderTarget.m_impl->reallocBufferIfNeeded(type, targetBytes);
    bufferPtr      = mapBuffer(type, allocatedBytes);
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::reallocAndRemapVerticesIfNeeded(base::SizeT moreCount)
{
    reallocAndRemapBufferIfNeeded(GL_ARRAY_BUFFER,
                                  m_mappedVertices,
                                  m_allocatedVertexBytes,
                                  sizeof(Vertex) * (m_vertexCount + moreCount));
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::reallocAndRemapIndicesIfNeeded(base::SizeT moreCount)
{
    reallocAndRemapBufferIfNeeded(GL_ELEMENT_ARRAY_BUFFER,
                                  m_mappedIndices,
                                  m_allocatedIndexBytes,
                                  sizeof(IndexType) * (m_indexCount + moreCount));
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::appendPreTransformedVertices(const Vertex* data, base::SizeT count, const Transform& transform)
{
    reallocAndRemapVerticesIfNeeded(count);
    auto* asVertexPtr = reinterpret_cast<Vertex*>(m_mappedVertices) + m_vertexCount;

    for (base::SizeT i = 0u; i < count; ++i)
    {
        asVertexPtr[i].position  = transform.transformPoint(data[i].position);
        asVertexPtr[i].color     = data[i].color;
        asVertexPtr[i].texCoords = data[i].texCoords;
    }

    m_vertexCount += count;
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::add(const Sprite& sprite)
{
    reallocAndRemapIndicesIfNeeded(6u);
    auto* asIndexPtr = reinterpret_cast<IndexType*>(m_mappedIndices) + m_indexCount;

    // Triangle strip: triangle #0
    asIndexPtr[0] = static_cast<IndexType>(m_vertexCount + 0u);
    asIndexPtr[1] = static_cast<IndexType>(m_vertexCount + 1u);
    asIndexPtr[2] = static_cast<IndexType>(m_vertexCount + 2u);

    // Triangle strip: triangle #1
    asIndexPtr[3] = static_cast<IndexType>(m_vertexCount + 1u);
    asIndexPtr[4] = static_cast<IndexType>(m_vertexCount + 2u);
    asIndexPtr[5] = static_cast<IndexType>(m_vertexCount + 3u);

    m_indexCount += 6u;

    reallocAndRemapVerticesIfNeeded(4u);
    sprite.getPreTransformedVertices(reinterpret_cast<Vertex*>(m_mappedVertices) + m_vertexCount);
    m_vertexCount += 4u;
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::addSubsequentIndices(base::SizeT count)
{
    reallocAndRemapIndicesIfNeeded(count);

    auto* asIndexPtr = reinterpret_cast<IndexType*>(m_mappedIndices) + m_indexCount;

    for (IndexType i = 0u; i < static_cast<IndexType>(count); ++i)
        asIndexPtr[i] = static_cast<IndexType>(m_vertexCount + i);

    m_indexCount += count;
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::clear()
{
    m_vertexCount = 0u;
    m_indexCount  = 0u;

    m_renderTarget.m_impl->vbo.bind();
    m_renderTarget.m_impl->ebo.bind();
}


////////////////////////////////////////////////////////////
void MappedDrawableBatch::draw(const RenderStates& renderStates)
{
    SFML_BASE_ASSERT(m_mappedVertices != nullptr);
    SFML_BASE_ASSERT(m_mappedIndices != nullptr);

    unmapBuffer(GL_ARRAY_BUFFER);
    m_mappedVertices = nullptr;

    unmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    m_mappedIndices = nullptr;

    m_renderTarget.drawMappedIndexedVertices(PrimitiveType::Triangles, m_indexCount, renderStates);
}
