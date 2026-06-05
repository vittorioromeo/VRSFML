#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "IndexUtils.hpp"
#include "ShapeDimension.hpp"
#include "Tetramino.hpp"

#include "ExampleUtils/ControlFlow.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Container/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
class [[nodiscard]] BlockGrid
{
private:
    ////////////////////////////////////////////////////////////
    za::Vector<za::Optional<Block>> m_blocks;
    za::SizeT                       m_width;
    za::SizeT                       m_height;


public:
    ////////////////////////////////////////////////////////////
    explicit BlockGrid(const za::SizeT width, const za::SizeT height) :
        m_blocks(width * height),
        m_width{width},
        m_height{height}
    {
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isInBounds(const za::Vec2<T> position) const
    {
        return position.x >= T{0} && position.x < static_cast<T>(m_width) && position.y >= T{0} &&
               position.y < static_cast<T>(m_height);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] za::Optional<Block>& at(const za::Vec2i position)
    {
        ZA_ASSERT(position.x >= 0);
        ZA_ASSERT(position.y >= 0);

        return at(position.toVec2uz());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const za::Optional<Block>& at(const za::Vec2i position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] za::Optional<Block>& at(const za::Vec2u position)
    {
        return at(position.toVec2uz());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const za::Optional<Block>& at(const za::Vec2u position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] za::Optional<Block>& at(const za::Vec2uz position)
    {
        ZA_ASSERT(position.x < m_width);
        ZA_ASSERT(position.y < m_height);

        const auto index = getIndex2Dto1D(position, m_width);
        ZA_ASSERT(index < m_blocks.size());

        return m_blocks[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const za::Optional<Block>& at(const za::Vec2uz position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] za::SizeT getWidth() const noexcept
    {
        return m_width;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] za::SizeT getHeight() const noexcept
    {
        return m_height;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isValidMove(const BlockMatrix& blockMatrix, const za::Vec2i newPosition) const
    {
        for (za::SizeT y = 0u; y < shapeDimension; ++y)
            for (za::SizeT x = 0u; x < shapeDimension; ++x)
            {
                if (!blockMatrix.at(x, y).hasValue())
                    continue;

                const auto gridPos = newPosition + za::Vec2uz{x, y}.toVec2i();

                if (!isInBounds(gridPos))
                    return false;

                if (at(gridPos).hasValue())
                    return false;
            }

        return true;
    }


    ////////////////////////////////////////////////////////////
    void embedTetramino(const Tetramino& tetramino)
    {
        for (za::SizeT y = 0u; y < shapeDimension; ++y)
            for (za::SizeT x = 0u; x < shapeDimension; ++x)
            {
                if (!tetramino.shape.at(x, y).hasValue())
                    continue;

                const auto gridPos = tetramino.position + za::Vec2uz{x, y}.toVec2i();

                if (!isInBounds(gridPos))
                    continue;

                ZA_ASSERT(!at(gridPos).hasValue());
                at(gridPos) = tetramino.shape.at(x, y);
            }
    }


    ////////////////////////////////////////////////////////////
    void shiftRowDown(const za::SizeT rowIndex)
    {
        ZA_ASSERT(rowIndex < m_height);

        // shift all rows down to cover the removed row
        for (za::SizeT y = rowIndex; y > 0; --y)
            for (za::SizeT x = 0u; x < m_width; ++x)
                at(za::Vec2uz{x, y}) = at(za::Vec2uz{x, y - 1});

        // clear the top row
        for (za::SizeT x = 0u; x < m_width; ++x)
            at(za::Vec2uz{x, 0u}).reset();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void forBlocks(auto&& func)
    {
        for (za::SizeT y = 0u; y < m_height; ++y)
            for (za::SizeT x = 0u; x < m_width; ++x)
                if (auto& optBlock = at(za::Vec2uz{x, y}); optBlock.hasValue())
                    if (func(*optBlock, za::Vec2uz{x, y}) == ControlFlow::Break)
                        return;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] auto& getBlocks() noexcept
    {
        return m_blocks;
    }
};

} // namespace tsurv
