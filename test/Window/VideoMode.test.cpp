#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/VideoMode.hpp"

#include "Zancle/Window/VideoModeUtils.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Algorithm/IsSorted.hpp"
#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[Window] za::VideoMode" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_DEFAULT_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::VideoMode));

        STATIC_CHECK(!ZB_IS_TRIVIAL(za::VideoMode)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::VideoMode));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::VideoMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::VideoMode, za::VideoMode));
    }

    SECTION("Construction")
    {
        SECTION("Width, height constructor")
        {
            const za::VideoMode videoMode{.size{800u, 600u}};
            CHECK(videoMode.size == za::Vec2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 32);
            CHECK(videoMode.pixelDensity == 1.f);
            CHECK(videoMode.refreshRate == 60.f);
        }

        SECTION("Width, height, bit depth constructor")
        {
            const za::VideoMode videoMode{.size{800u, 600u}, .bitsPerPixel = 24u};
            CHECK(videoMode.size == za::Vec2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 24);
            CHECK(videoMode.pixelDensity == 1.f);
            CHECK(videoMode.refreshRate == 60.f);
        }
    }

    SECTION("getFullscreenModes()")
    {
        const auto& modes = za::VideoModeUtils::getFullscreenModes();
        CHECK(zb::isSorted(modes.begin(), modes.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; }));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} == za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} ==
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }

        SECTION("operator!=")
        {
            CHECK(za::VideoMode{.size = {720, 720}} != za::VideoMode{.size = {720, 720}, .bitsPerPixel = 24});
            CHECK(za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 16} != za::VideoMode{.size = {400, 600}});
        }

        SECTION("operator<")
        {
            CHECK(za::VideoMode{.size = {800, 800}, .bitsPerPixel = 24} <
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} <
                  za::VideoMode{.size = {600, 400}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 400}, .bitsPerPixel = 48} <
                  za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
        }

        SECTION("operator>")
        {
            CHECK(za::VideoMode{.size = {1, 0}} > za::VideoMode{.size = {0, 0}, .bitsPerPixel = 1});
            CHECK(za::VideoMode{.size = {800, 800}, .bitsPerPixel = 48} >
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 24});
            CHECK(za::VideoMode{.size = {600, 400}, .bitsPerPixel = 48} >
                  za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} >
                  za::VideoMode{.size = {400, 400}, .bitsPerPixel = 48});
        }

        SECTION("operator<=")
        {
            CHECK(za::VideoMode{.size = {800, 800}, .bitsPerPixel = 24} <=
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} <=
                  za::VideoMode{.size = {600, 400}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 400}, .bitsPerPixel = 48} <=
                  za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} <= za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} <=
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }

        SECTION("operator>=")
        {
            CHECK(za::VideoMode{.size = {1, 0}} >= za::VideoMode{.size = {0, 0}, .bitsPerPixel = 1});
            CHECK(za::VideoMode{.size = {800, 800}, .bitsPerPixel = 48} >=
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 24});
            CHECK(za::VideoMode{.size = {600, 400}, .bitsPerPixel = 48} >=
                  za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} >=
                  za::VideoMode{.size = {400, 400}, .bitsPerPixel = 48});
            CHECK(za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} >= za::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} >=
                  za::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }
    }
}
