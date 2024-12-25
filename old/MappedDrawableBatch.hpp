
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API MappedDrawableBatch
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    using IndexType = unsigned int;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename BatchableObject>
    [[gnu::always_inline, gnu::flatten]] void add(const BatchableObject& batchableObject)
        requires(!base::isBaseOf<Shape, BatchableObject>)
    {
        const auto [data, size] = batchableObject.getVertices();

        addSubsequentIndices(size);
        appendTransformedVertices(data, size, batchableObject.getTransform());
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(const Sprite& sprite);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    void draw(const RenderStates& renderStates);

    MappedDrawableBatch(RenderTarget& renderTarget);

private:
    friend RenderTarget;

    [[gnu::always_inline, gnu::flatten]] void reallocAndRemapBufferIfNeeded(
        unsigned int type,
        void*&       bufferPtr,
        base::SizeT& allocatedBytes,
        base::SizeT  targetBytes);

    void reallocAndRemapVerticesIfNeeded(base::SizeT moreCount);
    void reallocAndRemapIndicesIfNeeded(base::SizeT moreCount);

    [[nodiscard, gnu::always_inline, gnu::flatten]] void* mapBuffer(unsigned int type, base::SizeT allocatedBytes) const;
    [[gnu::always_inline, gnu::flatten]] void unmapBuffer(unsigned int type) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void addSubsequentIndices(base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void appendTransformedVertices(const Vertex*    data,
                                                                           base::SizeT      count,
                                                                           const Transform& transform);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    RenderTarget& m_renderTarget;
    base::SizeT   m_allocatedVertexBytes{0u};
    base::SizeT   m_allocatedIndexBytes{0u};
    base::SizeT   m_vertexCount{0u};
    base::SizeT   m_indexCount{0u};
    void*         m_mappedVertices{};
    void*         m_mappedIndices{};
};
