#include "SFML/System/RectPacker.hpp"

#include "SFML/System/Vec2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>


namespace
{
////////////////////////////////////////////////////////////
void checkPack(sf::RectPacker& rectPacker, sf::Vec2u size, sf::Vec2u position)
{
    const auto p0 = rectPacker.pack(size);
    CHECK(p0.hasValue());
    CHECK(p0->x == position.x);
    CHECK(p0->y == position.y);
}

} // namespace

TEST_CASE("[System] sf::RectPacker", "")
{
    SECTION("Size constructor")
    {
        sf::RectPacker rectPacker({128u, 128u});
        CHECK(rectPacker.getSize() == sf::Vec2u{128u, 128u});
    }

    SECTION("Failure to pack -- OOB")
    {
        sf::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.pack({256u, 256u}));
    }

    SECTION("Failure to pack -- zero-sized")
    {
        sf::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.pack({0u, 256u}));
        CHECK(!rectPacker.pack({256u, 0u}));
    }

    SECTION("Pack -- 1x1")
    {
        sf::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {1u, 1u}, {0u, 0u});
        CHECK(!rectPacker.pack({128u, 128u}));
    }

    SECTION("Pack -- 128x128")
    {
        sf::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {128u, 128u}, {0u, 0u});
        CHECK(!rectPacker.pack({1u, 1u}));
    }

    SECTION("Pack -- 64x64")
    {
        sf::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {64u, 64u}, {0u, 0u});
        checkPack(rectPacker, {64u, 64u}, {64u, 0u});
        checkPack(rectPacker, {64u, 64u}, {0u, 64u});
        checkPack(rectPacker, {64u, 64u}, {64u, 64u});

        CHECK(!rectPacker.pack({1u, 1u}));
        CHECK(!rectPacker.pack({64u, 64u}));
    }

    SECTION("Pack Multiple -- OK")
    {
        const sf::Vec2u sizes[] = {{64u, 64u}, {64u, 64u}, {64u, 64u}, {64u, 64u}};
        sf::Vec2u       positions[4];

        sf::RectPacker rectPacker({128u, 128u});
        CHECK(rectPacker.packMultiple(positions, sizes));

        sf::base::Optional<sf::Vec2u> toMatch[4] = {
            sf::base::Optional<sf::Vec2u>(sf::base::inPlace, 0u, 0u),
            sf::base::Optional<sf::Vec2u>(sf::base::inPlace, 64u, 0u),
            sf::base::Optional<sf::Vec2u>(sf::base::inPlace, 0u, 64u),
            sf::base::Optional<sf::Vec2u>(sf::base::inPlace, 64u, 64u),
        };

        const auto findAndErase = [&](const sf::Vec2u& pos)
        {
            for (auto& i : toMatch)
                if (i.hasValue() && *i == pos)
                {
                    i = sf::base::nullOpt;
                    return true;
                }

            return false;
        };

        CHECK(findAndErase(sf::Vec2u{64u, 64u}));
        CHECK(findAndErase(sf::Vec2u{0u, 0u}));
        CHECK(findAndErase(sf::Vec2u{64u, 0u}));
        CHECK(findAndErase(sf::Vec2u{0u, 64u}));

        for (const auto& i : toMatch)
            CHECK(!i.hasValue());
    }

    SECTION("Pack Multiple -- Failure")
    {
        const sf::Vec2u sizes[] = {{64u, 64u}, {64u, 64u}, {64u, 64u}, {65u, 64u}};
        sf::Vec2u       positions[4];

        sf::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.packMultiple(positions, sizes));
    }
}
