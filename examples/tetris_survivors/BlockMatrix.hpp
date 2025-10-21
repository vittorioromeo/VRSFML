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
[[nodiscard]] inline BlockMatrix rotateBlockMatrixClockwise(const BlockMatrix& originalMatrix)
{
    BlockMatrix rotatedMatrix;

    for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
        for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            rotatedMatrix[x * shapeDimension + (shapeDimension - 1u - y)] = originalMatrix[y * shapeDimension + x];

    return rotatedMatrix;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline BlockMatrix rotateBlockMatrixCounterClockwise(const BlockMatrix& originalMatrix)
{
    BlockMatrix rotatedMatrix;

    for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
        for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            rotatedMatrix[(shapeDimension - 1u - x) * shapeDimension + y] = originalMatrix[y * shapeDimension + x];

    return rotatedMatrix;
}

} // namespace tsurv
