#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockMatrix.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] Tetramino // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   shape;
    sf::Vec2i     position;
    TetraminoType tetraminoType;
    sf::base::U8  rotationState; // [0-3]
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline BlockMatrix mapBlocksToNewShape(const Tetramino& tetramino, const ShapeMatrix& targetShapeTemplate)
{
    sf::base::Array<Block, 4> blockMap; // NOLINT(cppcoreguidelines-pro-type-member-init)

    for (const auto& block : tetramino.shape)
    {
        if (!block.hasValue())
            continue;

        const ShapeBlockSequence id = block->shapeBlockSequence;
        SFML_BASE_ASSERT(id != ShapeBlockSequence::_);

        blockMap[static_cast<sf::base::SizeT>(id) - 1u] = *block;
    }

    SFML_BASE_ASSERT(blockMap[0].shapeBlockSequence == ShapeBlockSequence::A);
    SFML_BASE_ASSERT(blockMap[1].shapeBlockSequence == ShapeBlockSequence::B);
    SFML_BASE_ASSERT(blockMap[2].shapeBlockSequence == ShapeBlockSequence::C);
    SFML_BASE_ASSERT(blockMap[3].shapeBlockSequence == ShapeBlockSequence::D);

    BlockMatrix newBlockMatrix;

    for (sf::base::SizeT i = 0u; i < newBlockMatrix.size(); ++i)
        if (const ShapeBlockSequence id = targetShapeTemplate[i]; id != ShapeBlockSequence::_)
            newBlockMatrix[i].emplace(blockMap[static_cast<sf::base::SizeT>(id) - 1u]);

    return newBlockMatrix;
}

} // namespace tsurv
