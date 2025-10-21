#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "ShapeDimension.hpp"
#include "Tetramino.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::base::SizeT getIndex2Dto1D(const sf::Vec2u position, const sf::base::SizeT width)
{
    return position.y * width + position.x;
}


////////////////////////////////////////////////////////////
class BlockGrid
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
    [[nodiscard]] sf::base::Optional<Block>& at(const sf::Vec2u position)
    {
        SFML_BASE_ASSERT(position.x < m_width);
        SFML_BASE_ASSERT(position.y < m_height);

        const auto index = getIndex2Dto1D(position, m_width);
        SFML_BASE_ASSERT(index < m_blocks.size());

        return m_blocks[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::base::Optional<Block>& at(const sf::Vec2u position) const
    {
        return const_cast<BlockGrid*>(this)->at(position);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT getWidth() const noexcept
    {
        return m_width;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT getHeight() const noexcept
    {
        return m_height;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isValidMove(const BlockMatrix& blockMatrix, const sf::Vec2i newPosition) const
    {
        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
            for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            {
                if (!blockMatrix[y * shapeDimension + x].hasValue())
                    continue;

                const auto gridPos = newPosition + sf::Vec2uz{x, y}.toVec2i();

                if (gridPos.x < 0 || gridPos.x >= static_cast<int>(m_width) || gridPos.y < 0 ||
                    gridPos.y >= static_cast<int>(m_height))
                    return false;

                if (at(gridPos.toVec2u()).hasValue())
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
                const auto index = y * shapeDimension + x;

                if (!tetramino.shape[index].hasValue())
                    continue;

                const auto gridPos = tetramino.position + sf::Vec2uz{x, y}.toVec2i();

                if (gridPos.x >= 0 && gridPos.x < static_cast<int>(m_width) && gridPos.y >= 0 &&
                    gridPos.y < static_cast<int>(m_height))
                {
                    SFML_BASE_ASSERT(!at(gridPos.toVec2u()).hasValue());
                    at(gridPos.toVec2u()) = tetramino.shape[index];
                }
            }
    }
};

} // namespace tsurv
