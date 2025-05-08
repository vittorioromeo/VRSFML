#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class RenderTarget;
class Shape;
class Text;

struct ArrowShapeData;
struct CircleShapeData;
struct EllipseShapeData;
struct PieSliceShapeData;
struct RectangleShapeData;
struct RingShapeData;
struct RingPieSliceShapeData;
struct RoundedRectangleShapeData;
struct StarShapeData;
struct Sprite;
struct TextData;
struct Transform;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct CPUStorage
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear()
    {
        vertices.clear();
        indices.clear();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] Vertex* reserveMoreVertices(const base::SizeT count)
    {
        return vertices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType* reserveMoreIndices(const base::SizeT count)
    {
        return indices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreVertices(const base::SizeT count) noexcept
    {
        vertices.unsafeSetSize(vertices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreIndices(const base::SizeT count) noexcept
    {
        indices.unsafeSetSize(indices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return static_cast<IndexType>(vertices.size());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return static_cast<IndexType>(indices.size());
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::Vector<Vertex>    vertices; //!< CPU buffer for vertices
    base::Vector<IndexType> indices;  //!< CPU buffer for indices
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct PersistentGPUStorage
{
    ////////////////////////////////////////////////////////////
    explicit PersistentGPUStorage();
    ~PersistentGPUStorage();

    ////////////////////////////////////////////////////////////
    PersistentGPUStorage(const PersistentGPUStorage&)            = delete;
    PersistentGPUStorage& operator=(const PersistentGPUStorage&) = delete;

    ////////////////////////////////////////////////////////////
    PersistentGPUStorage(PersistentGPUStorage&&) noexcept;
    PersistentGPUStorage& operator=(PersistentGPUStorage&&) noexcept;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        nVertices = nIndices = 0u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vertex*    reserveMoreVertices(base::SizeT count);
    [[nodiscard]] IndexType* reserveMoreIndices(base::SizeT count);

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreVertices(const base::SizeT count) noexcept
    {
        nVertices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreIndices(const base::SizeT count) noexcept
    {
        nIndices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return nVertices;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return nIndices;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const void* getVAOGroup() const;

    ////////////////////////////////////////////////////////////
    void flushVertexWritesToGPU(base::SizeT count, base::SizeT offset);

    ////////////////////////////////////////////////////////////
    void flushIndexWritesToGPU(base::SizeT count, base::SizeT offset);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> impl; //!< Implementation details

    IndexType nVertices{}; //!< Number of "active" vertices in the buffer
    IndexType nIndices{};  //!< Number of "active" indices in the buffer
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename TStorage>
class [[nodiscard]] SFML_GRAPHICS_API DrawableBatchImpl : public Transformable
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename... TStorageArgs>
    explicit DrawableBatchImpl(TStorageArgs&&... storageArgs) : m_storage(SFML_BASE_FORWARD(storageArgs)...)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reserveTriangles(const base::SizeT triangleCount)
    {
        (void)m_storage.reserveMoreIndices(3u * triangleCount);
        (void)m_storage.reserveMoreVertices(3u * triangleCount);
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reserveQuads(const base::SizeT quadCount)
    {
        (void)m_storage.reserveMoreIndices(6u * quadCount);
        (void)m_storage.reserveMoreVertices(4u * quadCount);
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    /// \note Only supports triangle primitives.
    ///
    ////////////////////////////////////////////////////////////
    void add(const Vertex* vertexData, base::SizeT vertexCount, PrimitiveType type);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    /// \note Only supports triangle primitives.
    ///
    ////////////////////////////////////////////////////////////
    void add(const Vertex*    vertexData,
             base::SizeT      vertexCount,
             const IndexType* indexData,
             base::SizeT      indexCount,
             PrimitiveType    type);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(const Sprite& sprite);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(const Shape& shape);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(const Text& text);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const ArrowShapeData& sdArrow);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const CircleShapeData& sdCircle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const EllipseShapeData& sdEllipse);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const PieSliceShapeData& sdPieSlice);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RectangleShapeData& sdRectangle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RingShapeData& sdRing);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RingPieSliceShapeData& sdRingPieSlice);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RoundedRectangleShapeData& sdRoundedRectangle);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const StarShapeData& sdStarShape);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const Font& font, const TextData& textData);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        m_storage.clear();
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isEmpty() const noexcept
    {
        return m_storage.getNumVertices() == 0u && m_storage.getNumIndices() == 0u;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return m_storage.getNumVertices();
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return m_storage.getNumIndices();
    }

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan drawTriangleFanShapeFromPoints(base::SizeT nPoints,
                                              const auto& descriptor,
                                              auto&&      pointFn,
                                              Vec2f       centerOffset = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void addShapeFill(const Transform& transform, const Vertex* data, base::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void addShapeOutline(const Transform& transform, const Vertex* data, base::SizeT size);

protected:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    TStorage m_storage;
};

////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class DrawableBatchImpl<CPUStorage>;
extern template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class CPUDrawableBatch : public priv::DrawableBatchImpl<priv::CPUStorage>
{
    using DrawableBatchImpl<priv::CPUStorage>::DrawableBatchImpl;
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class PersistentGPUDrawableBatch : public priv::DrawableBatchImpl<priv::PersistentGPUStorage>
{
    using DrawableBatchImpl<priv::PersistentGPUStorage>::DrawableBatchImpl;

public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset)
    {
        m_storage.flushVertexWritesToGPU(count, offset);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset)
    {
        m_storage.flushIndexWritesToGPU(count, offset);
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::DrawableBatch
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
