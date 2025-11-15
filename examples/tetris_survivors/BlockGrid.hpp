#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "ExampleUtils/ControlFlow.hpp"
#include "IndexUtils.hpp"
#include "ShapeDimension.hpp"
#include "Tetramino.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
class [[nodiscard]] BlockGrid
{
private:
    ////////////////////////////////////////////////////////////
    sf::base::Vector<sf::base::Optional<Block>> m_blocks;
    sf::base::SizeT                             m_width;
    sf::base::SizeT                             m_height;


public:
    ////////////////////////////////////////////////////////////
    explicit BlockGrid(const sf::base::SizeT width, const sf::base::SizeT height) :
        m_blocks(width * height),
        m_width{width},
        m_height{height}
    {
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isInBounds(const sf::Vec2<T> position) const
    {
        return position.x >= T{0} && position.x < static_cast<T>(m_width) && position.y >= T{0} &&
               position.y < static_cast<T>(m_height);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] sf::base::Optional<Block>& at(const sf::Vec2i position)
    {
        SFML_BASE_ASSERT(position.x >= 0);
        SFML_BASE_ASSERT(position.y >= 0);

        return at(position.toVec2uz());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const sf::base::Optional<Block>& at(const sf::Vec2i position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] sf::base::Optional<Block>& at(const sf::Vec2u position)
    {
        return at(position.toVec2uz());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const sf::base::Optional<Block>& at(const sf::Vec2u position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] sf::base::Optional<Block>& at(const sf::Vec2uz position)
    {
        SFML_BASE_ASSERT(position.x < m_width);
        SFML_BASE_ASSERT(position.y < m_height);

        const auto index = getIndex2Dto1D(position, m_width);
        SFML_BASE_ASSERT(index < m_blocks.size());

        return m_blocks[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const sf::base::Optional<Block>& at(const sf::Vec2uz position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] sf::base::SizeT getWidth() const noexcept
    {
        return m_width;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] sf::base::SizeT getHeight() const noexcept
    {
        return m_height;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isValidMove(const BlockMatrix& blockMatrix, const sf::Vec2i newPosition) const
    {
        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
            for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            {
                if (!blockMatrix.at(x, y).hasValue())
                    continue;

                const auto gridPos = newPosition + sf::Vec2uz{x, y}.toVec2i();

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
        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
            for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            {
                if (!tetramino.shape.at(x, y).hasValue())
                    continue;

                const auto gridPos = tetramino.position + sf::Vec2uz{x, y}.toVec2i();

                if (!isInBounds(gridPos))
                    continue;

                SFML_BASE_ASSERT(!at(gridPos).hasValue());
                at(gridPos) = tetramino.shape.at(x, y);
            }
    }


    ////////////////////////////////////////////////////////////
    void shiftRowDown(const sf::base::SizeT rowIndex)
    {
        SFML_BASE_ASSERT(rowIndex < m_height);

        // shift all rows down to cover the removed row
        for (sf::base::SizeT y = rowIndex; y > 0; --y)
            for (sf::base::SizeT x = 0u; x < m_width; ++x)
                at(sf::Vec2uz{x, y}) = at(sf::Vec2uz{x, y - 1});

        // clear the top row
        for (sf::base::SizeT x = 0u; x < m_width; ++x)
            at(sf::Vec2uz{x, 0u}).reset();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void forBlocks(auto&& func)
    {
        for (sf::base::SizeT y = 0u; y < m_height; ++y)
            for (sf::base::SizeT x = 0u; x < m_width; ++x)
                if (auto& optBlock = at(sf::Vec2uz{x, y}); optBlock.hasValue())
                    if (func(*optBlock, sf::Vec2uz{x, y}) == ControlFlow::Break)
                        return;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] auto& getBlocks() noexcept
    {
        return m_blocks;
    }
};

} // namespace tsurv
