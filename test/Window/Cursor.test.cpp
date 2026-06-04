#include "StringifyOptionalUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/Cursor.hpp"

#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"

TEST_CASE("[Window] za::Cursor" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_DEFAULT_CONSTRUCTIBLE(za::Cursor));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::Cursor));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::Cursor));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Cursor));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Cursor));
    }

    SECTION("loadFromPixels()")
    {
        static constexpr zb::U8 pixels[4]{};

        CHECK(!za::Cursor::loadFromPixels(nullptr, {}, {}).hasValue());
        CHECK(!za::Cursor::loadFromPixels(pixels, {0, 1}, {}).hasValue());
        CHECK(!za::Cursor::loadFromPixels(pixels, {1, 0}, {}).hasValue());
        CHECK(za::Cursor::loadFromPixels(pixels, {1, 1}, {}).hasValue());
    }

    SECTION("loadFromSystem()")
    {
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::Hand).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeHorizontal).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeVertical).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeLeft).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeRight).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeTop).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeBottom).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeTopLeft).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeTopRight).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeBottomLeft).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::SizeBottomRight).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::Cross).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::Help).hasValue());
        CHECK(za::Cursor::loadFromSystem(za::Cursor::Type::NotAllowed).hasValue());
    }
}
