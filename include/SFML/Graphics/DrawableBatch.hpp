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
class PersistentGPUBuffer;
class RenderTarget;
class Shape;
class Text;
class Transform;
struct Sprite;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
using IndexType = unsigned int;


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct PersistentGPUStorage
{
    explicit PersistentGPUStorage(RenderTarget& renderTarget);

    PersistentGPUBuffer& verticesPersistentBuffer;
    PersistentGPUBuffer& indicesPersistentBuffer;

    IndexType nVertices{};
    IndexType nIndices{};

    void clear();

    [[nodiscard]] Vertex*    reserveMoreVerticesAndGetPtr(base::SizeT count);
    [[nodiscard]] IndexType* reserveMoreIndicesAndGetPtr(base::SizeT count);

    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType getNumVertices() const
    {
        return nVertices;
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType getNumIndices() const
    {
        return nIndices;
    }
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct CPUStorage
{
    base::TrivialVector<Vertex>    vertices; //!< TODO P0:
    base::TrivialVector<IndexType> indices;  //!< TODO P0:

    void clear();

    [[nodiscard]] Vertex*    reserveMoreVerticesAndGetPtr(base::SizeT count);
    [[nodiscard]] IndexType* reserveMoreIndicesAndGetPtr(base::SizeT count);

    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType getNumVertices() const
    {
        return static_cast<IndexType>(vertices.size());
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType getNumIndices() const
    {
        return static_cast<IndexType>(indices.size());
    }
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
extern template class DrawableBatchImpl<CPUStorage>;
extern template class DrawableBatchImpl<PersistentGPUStorage>;


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class CPUDrawableBatch : public DrawableBatchImpl<CPUStorage>
{
    friend RenderTarget;
    using DrawableBatchImpl<CPUStorage>::DrawableBatchImpl;
};


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class PersistentGPUDrawableBatch : public DrawableBatchImpl<PersistentGPUStorage>
{
    friend RenderTarget;
    using DrawableBatchImpl<PersistentGPUStorage>::DrawableBatchImpl;
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
