#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
template <typename TBufferObject>
class GLPersistentBuffer;

class RenderTarget;
class Shape;
class Text;
struct GLElementBufferObject;
struct GLVertexBufferObject;
struct Sprite;
struct Transform;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
using IndexType = unsigned int;

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
    [[nodiscard, gnu::always_inline, gnu::flatten]] Vertex* reserveMoreVertices(base::SizeT count)
    {
        return vertices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType* reserveMoreIndices(base::SizeT count)
    {
        return indices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreVertices(base::SizeT count)
    {
        vertices.unsafeSetSize(vertices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreIndices(base::SizeT count)
    {
        indices.unsafeSetSize(indices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const
    {
        return static_cast<IndexType>(vertices.size());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const
    {
        return static_cast<IndexType>(indices.size());
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::TrivialVector<Vertex>    vertices; //!< CPU buffer for vertices
    base::TrivialVector<IndexType> indices;  //!< CPU buffer for indices
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct PersistentGPUStorage
{
    ////////////////////////////////////////////////////////////
    explicit PersistentGPUStorage(RenderTarget& renderTarget);

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        nVertices = nIndices = 0u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vertex*    reserveMoreVertices(base::SizeT count);
    [[nodiscard]] IndexType* reserveMoreIndices(base::SizeT count);

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreVertices(base::SizeT count)
    {
        nVertices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreIndices(base::SizeT count)
    {
        nIndices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const
    {
        return nVertices;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const
    {
        return nIndices;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    GLPersistentBuffer<GLVertexBufferObject>&  vboPersistentBuffer; //!< GPU persistent buffer for vertices
    GLPersistentBuffer<GLElementBufferObject>& eboPersistentBuffer; //!< GPU persistent buffer for indices

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
    void addTriangles(const Transform& transform, const Vertex* data, base::SizeT size);

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
    void clear();

private:
    friend RenderTarget;

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
