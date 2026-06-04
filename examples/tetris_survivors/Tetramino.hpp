#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockMatrix.hpp"
#include "StrongTypedef.hpp"
#include "TetraminoShapes.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
TSURV_DEFINE_STRONG_TYPEDEF(RotationState, zb::U8);


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
    zb::Array<Block, 4> blockMap; // NOLINT(cppcoreguidelines-pro-type-member-init)

    for (const auto& block : tetramino.shape.data)
    {
        if (!block.hasValue())
            continue;

        const ShapeBlockSequence id = block->shapeBlockSequence;
        ZB_ASSERT(id != ShapeBlockSequence::_);

        blockMap[static_cast<zb::SizeT>(id) - 1u] = *block;
    }

    ZB_ASSERT(blockMap[0].shapeBlockSequence == ShapeBlockSequence::A);
    ZB_ASSERT(blockMap[1].shapeBlockSequence == ShapeBlockSequence::B);
    ZB_ASSERT(blockMap[2].shapeBlockSequence == ShapeBlockSequence::C);
    ZB_ASSERT(blockMap[3].shapeBlockSequence == ShapeBlockSequence::D);

    BlockMatrix newBlockMatrix;

    for (zb::SizeT i = 0u; i < newBlockMatrix.data.size(); ++i)
        if (const ShapeBlockSequence id = targetShapeTemplate[i]; id != ShapeBlockSequence::_)
            newBlockMatrix.data[i].emplace(blockMap[static_cast<zb::SizeT>(id) - 1u]);

    return newBlockMatrix;
}

} // namespace tsurv
