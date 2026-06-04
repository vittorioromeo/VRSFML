#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/Glyph.hpp"

#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::Glyph")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::Glyph));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::Glyph));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::Glyph));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::Glyph));
    }

    SECTION("Construction")
    {
        constexpr za::Glyph glyph{};
        STATIC_CHECK(glyph.advance == 0.f);
        STATIC_CHECK(glyph.lsbDelta == 0);
        STATIC_CHECK(glyph.rsbDelta == 0);
        STATIC_CHECK(glyph.bounds == za::Rect2f());
        STATIC_CHECK(glyph.textureRect == za::Rect2f());
    }
}
