#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "ShapeDimension.hpp"

#include "SFML/Base/Array.hpp"
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
        if (const auto shapeBlock = shapeMatrix[i]; shapeBlock != ShapeBlock::_)
            blockMatrix[i].emplace(block).shapeBlock = shapeBlock;

    return blockMatrix;
}

} // namespace tsurv
