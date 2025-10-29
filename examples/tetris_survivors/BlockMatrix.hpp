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
using BlockMatrix = sf::base::Array<sf::base::Optional<Block>, shapeDimension * shapeDimension>;


////////////////////////////////////////////////////////////
[[nodiscard]] inline BlockMatrix shapeMatrixToBlockMatrix(const ShapeMatrix& shapeMatrix, const Block& block)
{
    BlockMatrix blockMatrix;

    for (sf::base::SizeT i = 0u; i < shapeMatrix.size(); ++i)
        if (const auto shapeBlockSequence = shapeMatrix[i]; shapeBlockSequence != ShapeBlockSequence::_)
            blockMatrix[i].emplace(block).shapeBlockSequence = shapeBlockSequence;

    return blockMatrix;
}


////////////////////////////////////////////////////////////
inline sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findDownmostBlocks(const BlockMatrix& shape)
{
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> result;

    bool foundLastRow = false;

    for (sf::base::SizeT y = shapeDimension; y-- > 0;)
    {
        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            if (shape[y * shapeDimension + x].hasValue())
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
inline sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findTopmostBlocks(const BlockMatrix& shape)
{
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> result;

    bool foundFirstRow = false;

    for (sf::base::SizeT y = 0; y < shapeDimension; ++y)
    {
        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            if (shape[y * shapeDimension + x].hasValue())
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
inline sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findHorizontalBlocks(const BlockMatrix&    shape,
                                                                                const sf::base::SizeT maxDepth)
{
    const auto idx = [](sf::base::SizeT x, sf::base::SizeT y) -> sf::base::SizeT
    { return getIndex2Dto1D(sf::Vec2uz{x, y}, shapeDimension); };

    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> result;

    sf::base::SizeT foundY = 0u;

    for (sf::base::SizeT y = shapeDimension; y-- > 0;)
    {
        sf::base::SizeT xCount = 0u;

        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
        {
            const auto iCenter = idx(x, y);
            const auto iLeft   = idx(x - 1, y);
            const auto iRight  = idx(x + 1, y);

            const bool leftEmpty  = (x == 0) || !shape[iLeft].hasValue();
            const bool rightEmpty = (x == shapeDimension - 1) || !shape[iRight].hasValue();

            if (shape[iCenter].hasValue() && leftEmpty && rightEmpty)
            {
                result.emplaceBack(x, y);
                ++foundY;
            }

            if (shape[iCenter].hasValue())
                ++xCount;
        }

        if (foundY >= maxDepth || xCount > 1u)
            break;
    }

    return result;
}

} // namespace tsurv
