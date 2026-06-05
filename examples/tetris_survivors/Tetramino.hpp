#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockMatrix.hpp"
#include "StrongTypedef.hpp"
#include "TetraminoShapes.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Base/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
TSURV_DEFINE_STRONG_TYPEDEF(RotationState, za::U8);


////////////////////////////////////////////////////////////
struct [[nodiscard]] Tetramino // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   shape;
    za::Vec2i     position;
    TetraminoType tetraminoType;
    RotationState rotationState; // [0-3]
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline BlockMatrix mapBlocksToNewShape(const Tetramino& tetramino, const ShapeMatrix& targetShapeTemplate)
{
    za::Array<Block, 4> blockMap; // NOLINT(cppcoreguidelines-pro-type-member-init)

    for (const auto& block : tetramino.shape.data)
    {
        if (!block.hasValue())
            continue;

        const ShapeBlockSequence id = block->shapeBlockSequence;
        ZA_ASSERT(id != ShapeBlockSequence::_);

        blockMap[static_cast<za::SizeT>(id) - 1u] = *block;
    }

    ZA_ASSERT(blockMap[0].shapeBlockSequence == ShapeBlockSequence::A);
    ZA_ASSERT(blockMap[1].shapeBlockSequence == ShapeBlockSequence::B);
    ZA_ASSERT(blockMap[2].shapeBlockSequence == ShapeBlockSequence::C);
    ZA_ASSERT(blockMap[3].shapeBlockSequence == ShapeBlockSequence::D);

    BlockMatrix newBlockMatrix;

    for (za::SizeT i = 0u; i < newBlockMatrix.data.size(); ++i)
        if (const ShapeBlockSequence id = targetShapeTemplate[i]; id != ShapeBlockSequence::_)
            newBlockMatrix.data[i].emplace(blockMap[static_cast<za::SizeT>(id) - 1u]);

    return newBlockMatrix;
}

} // namespace tsurv
