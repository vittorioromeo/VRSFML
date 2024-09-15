#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsBaseOf.hpp"
#include "SFML/Base/TrivialVector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class RenderTarget;
class Shape;
class Sprite;
class Transform;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API DrawableBatch
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
    [[gnu::always_inline, gnu::flatten]] void add(RenderTarget& rt, const BatchableObject& batchableObject)
        requires(!base::isBaseOf<Shape, BatchableObject>)
    {
        const auto [data, size] = batchableObject.getVertices();

        addSubsequentIndices(rt, size);
        appendPreTransformedVertices(rt, data, size, batchableObject.getTransform());
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(RenderTarget& rt, const Sprite& sprite);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void add(RenderTarget& rt, const Shape& shape);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void clear();

private:
    friend RenderTarget;

    IndexType* reserveMoreIndicesAndGetPtr(RenderTarget& rt, base::SizeT count) const;
    Vertex*    reserveMoreVerticesAndGetPtr(RenderTarget& rt, base::SizeT count) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void addSubsequentIndices(RenderTarget& rt, base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void appendPreTransformedVertices(
        RenderTarget&    rt,
        const Vertex*    data,
        base::SizeT      count,
        const Transform& transform)
    {
        // m_vertices.reserveMore(count);
        auto* vertices = reserveMoreVerticesAndGetPtr(rt, count);

        for (const auto* const target = data + count; data != target; ++data)
            *vertices++ = Vertex{transform.transformPoint(data->position), data->color, data->texCoords};

        m_nVerts += count;
        // m_vertices.unsafeEmplaceBack();
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::SizeT m_nVerts{};
    base::SizeT m_nIdxs{};
    // base::TrivialVector<Vertex>    m_vertices; //!< TODO P0:
    // base::TrivialVector<IndexType> m_indices;  //!< TODO P0:
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::DrawableBatch
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::RenderTarget
///
////////////////////////////////////////////////////////////
