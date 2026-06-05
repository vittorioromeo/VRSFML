#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Geometry/RectPacker.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
void checkPack(za::RectPacker& rectPacker, za::Vec2u size, za::Vec2u position)
{
    const auto p0 = rectPacker.pack(size);
    CHECK(p0.hasValue());
    CHECK(p0->x == position.x);
    CHECK(p0->y == position.y);
}

} // namespace

TEST_CASE("[System] za::RectPacker", "")
{
    SECTION("Size constructor")
    {
        za::RectPacker rectPacker({128u, 128u});
        CHECK(rectPacker.getSize() == za::Vec2u{128u, 128u});
    }

    SECTION("Failure to pack -- OOB")
    {
        za::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.pack({256u, 256u}));
    }

    SECTION("Failure to pack -- zero-sized")
    {
        za::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.pack({0u, 256u}));
        CHECK(!rectPacker.pack({256u, 0u}));
    }

    SECTION("Pack -- 1x1")
    {
        za::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {1u, 1u}, {0u, 0u});
        CHECK(!rectPacker.pack({128u, 128u}));
    }

    SECTION("Pack -- 128x128")
    {
        za::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {128u, 128u}, {0u, 0u});
        CHECK(!rectPacker.pack({1u, 1u}));
    }

    SECTION("Pack -- 64x64")
    {
        za::RectPacker rectPacker({128u, 128u});

        checkPack(rectPacker, {64u, 64u}, {0u, 0u});
        checkPack(rectPacker, {64u, 64u}, {64u, 0u});
        checkPack(rectPacker, {64u, 64u}, {0u, 64u});
        checkPack(rectPacker, {64u, 64u}, {64u, 64u});

        CHECK(!rectPacker.pack({1u, 1u}));
        CHECK(!rectPacker.pack({64u, 64u}));
    }

    SECTION("Pack Multiple -- OK")
    {
        const za::Vec2u sizes[] = {{64u, 64u}, {64u, 64u}, {64u, 64u}, {64u, 64u}};
        za::Vec2u       positions[4];

        za::RectPacker rectPacker({128u, 128u});
        CHECK(rectPacker.packMultiple(positions, sizes));

        za::Optional<za::Vec2u> toMatch[4] = {
            za::Optional<za::Vec2u>(za::inPlace, 0u, 0u),
            za::Optional<za::Vec2u>(za::inPlace, 64u, 0u),
            za::Optional<za::Vec2u>(za::inPlace, 0u, 64u),
            za::Optional<za::Vec2u>(za::inPlace, 64u, 64u),
        };

        const auto findAndErase = [&](const za::Vec2u& pos)
        {
            for (auto& i : toMatch)
                if (i.hasValue() && *i == pos)
                {
                    i = za::nullOpt;
                    return true;
                }

            return false;
        };

        CHECK(findAndErase(za::Vec2u{64u, 64u}));
        CHECK(findAndErase(za::Vec2u{0u, 0u}));
        CHECK(findAndErase(za::Vec2u{64u, 0u}));
        CHECK(findAndErase(za::Vec2u{0u, 64u}));

        for (const auto& i : toMatch)
            CHECK(!i.hasValue());
    }

    SECTION("Pack Multiple -- Failure")
    {
        const za::Vec2u sizes[] = {{64u, 64u}, {64u, 64u}, {64u, 64u}, {65u, 64u}};
        za::Vec2u       positions[4];

        za::RectPacker rectPacker({128u, 128u});
        CHECK(!rectPacker.packMultiple(positions, sizes));
    }
}
