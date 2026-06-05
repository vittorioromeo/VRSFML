#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/VideoMode.hpp"

#include "Zancle/Window/VideoModeUtils.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Algorithm/IsSorted.hpp"
#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsDefaultConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"
#include "Zancle/Trait/IsStandardLayout.hpp"
#include "Zancle/Trait/IsTrivial.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[Window] za::VideoMode" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_DEFAULT_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::VideoMode));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::VideoMode)); // because of member initializers
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::VideoMode));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::VideoMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::VideoMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::VideoMode, za::VideoMode));
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
        CHECK(za::isSorted(modes.begin(), modes.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; }));
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
