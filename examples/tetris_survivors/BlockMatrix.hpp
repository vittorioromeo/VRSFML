#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "IndexUtils.hpp"
#include "ShapeDimension.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Optional.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] BlockMatrix
{
    ////////////////////////////////////////////////////////////
    sf::base::Array<sf::base::Optional<Block>, shapeDimension * shapeDimension> data;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<Block>& at(const sf::base::SizeT x, const sf::base::SizeT y)
    {
        SFML_BASE_ASSERT(x < shapeDimension);
        SFML_BASE_ASSERT(y < shapeDimension);

        const auto index = getIndex2Dto1D(sf::Vec2uz{x, y}, shapeDimension);
        SFML_BASE_ASSERT(index < data.size());

        return data[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::base::Optional<Block>& at(const sf::base::SizeT x, const sf::base::SizeT y) const
    {
        return const_cast<BlockMatrix*>(this)->at(x, y);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<Block>& at(const int x, const int y)
    {
        SFML_BASE_ASSERT(x >= 0);
        SFML_BASE_ASSERT(y >= 0);

        return at(static_cast<sf::base::SizeT>(x), static_cast<sf::base::SizeT>(y));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::base::Optional<Block>& at(const int x, const int y) const
    {
        return const_cast<BlockMatrix*>(this)->at(x, y);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isInBounds(const sf::base::SizeT x, const sf::base::SizeT y) const
    {
        return x < shapeDimension && y < shapeDimension;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isInBounds(const int x, const int y) const
    {
        return x >= 0 && x < static_cast<int>(shapeDimension) && y >= 0 && y < static_cast<int>(shapeDimension);
    }
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline BlockMatrix shapeMatrixToBlockMatrix(const ShapeMatrix& shapeMatrix, const Block& block)
{
    BlockMatrix blockMatrix;

    for (sf::base::SizeT i = 0u; i < shapeMatrix.size(); ++i)
        if (const auto shapeBlockSequence = shapeMatrix[i]; shapeBlockSequence != ShapeBlockSequence::_)
            blockMatrix.data[i].emplace(block).shapeBlockSequence = shapeBlockSequence;

    return blockMatrix;
}


////////////////////////////////////////////////////////////
using ShapeBlockPositionVector = sf::base::InPlaceVector<sf::Vec2uz, shapeDimension>;


////////////////////////////////////////////////////////////
inline ShapeBlockPositionVector findDownmostBlocks(const BlockMatrix& shape)
{
    ShapeBlockPositionVector result;

    bool foundLastRow = false;

    for (sf::base::SizeT y = shapeDimension; y-- > 0;)
    {
        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            if (shape.at(x, y).hasValue())
            {
                foundLastRow = true;
                result.emplaceBack(x, y);
            }
        }

        if (foundLastRow)
            break;
    }

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline ShapeBlockPositionVector findTopmostBlocks(const BlockMatrix& shape)
{
    ShapeBlockPositionVector result;

    bool foundFirstRow = false;

    for (sf::base::SizeT y = 0; y < shapeDimension; ++y)
    {
        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            if (shape.at(x, y).hasValue())
            {
                foundFirstRow = true;
                result.emplaceBack(x, y);
            }
        }

        if (foundFirstRow)
            break;
    }

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline ShapeBlockPositionVector findHorizontalBlocks(const BlockMatrix& shape, const sf::base::SizeT maxDepth)
{
    ShapeBlockPositionVector result;

    sf::base::SizeT foundY = 0u;

    for (sf::base::SizeT y = shapeDimension; y-- > 0;)
    {
        sf::base::SizeT xCount = 0u;

        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            const bool leftEmpty  = (x == 0) || !shape.at(x - 1, y).hasValue();
            const bool rightEmpty = (x == shapeDimension - 1) || !shape.at(x + 1, y).hasValue();

            if (shape.at(x, y).hasValue() && leftEmpty && rightEmpty)
            {
                result.emplaceBack(x, y);
                ++foundY;
            }

            if (shape.at(x, y).hasValue())
                ++xCount;
        }

        if (foundY >= maxDepth || xCount > 1u)
            break;
    }

    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] inline bool blockMatricesIntersect(
    const BlockMatrix& shape1,
    const sf::Vec2i    pos1,
    const BlockMatrix& shape2,
    const sf::Vec2i    pos2)
{
    for (sf::base::SizeT y1 = 0; y1 < shapeDimension; ++y1)
        for (sf::base::SizeT x1 = 0; x1 < shapeDimension; ++x1)
        {
            if (!shape1.at(x1, y1).hasValue())
                continue;

            const sf::Vec2i worldPos1 = pos1 + sf::Vec2uz{x1, y1}.toVec2i();

            for (sf::base::SizeT y2 = 0; y2 < shapeDimension; ++y2)
                for (sf::base::SizeT x2 = 0; x2 < shapeDimension; ++x2)
                {
                    if (!shape2.at(x2, y2).hasValue())
                        continue;

                    const sf::Vec2i worldPos2 = pos2 + sf::Vec2uz{x2, y2}.toVec2i();

                    if (worldPos1 == worldPos2)
                        return true;
                }
        }

    return false;
}

} // namespace tsurv
