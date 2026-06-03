#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "SFML/Window/VideoMode.hpp"

#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Algorithm/IsSorted.hpp"
#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[Window] sf::VideoMode" * tst::skip(skipDisplayTests))
{
    auto windowContext = sf::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::VideoMode));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::VideoMode)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::VideoMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::VideoMode, sf::VideoMode));
    }

    SECTION("Construction")
    {
        SECTION("Width, height constructor")
        {
            const sf::VideoMode videoMode{.size{800u, 600u}};
            CHECK(videoMode.size == sf::Vec2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 32);
            CHECK(videoMode.pixelDensity == 1.f);
            CHECK(videoMode.refreshRate == 60.f);
        }

        SECTION("Width, height, bit depth constructor")
        {
            const sf::VideoMode videoMode{.size{800u, 600u}, .bitsPerPixel = 24u};
            CHECK(videoMode.size == sf::Vec2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 24);
            CHECK(videoMode.pixelDensity == 1.f);
            CHECK(videoMode.refreshRate == 60.f);
        }
    }

    SECTION("getFullscreenModes()")
    {
        const auto& modes = sf::VideoModeUtils::getFullscreenModes();
        CHECK(sf::base::isSorted(modes.begin(), modes.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; }));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} == sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} ==
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }

        SECTION("operator!=")
        {
            CHECK(sf::VideoMode{.size = {720, 720}} != sf::VideoMode{.size = {720, 720}, .bitsPerPixel = 24});
            CHECK(sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 16} != sf::VideoMode{.size = {400, 600}});
        }

        SECTION("operator<")
        {
            CHECK(sf::VideoMode{.size = {800, 800}, .bitsPerPixel = 24} <
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} <
                  sf::VideoMode{.size = {600, 400}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 400}, .bitsPerPixel = 48} <
                  sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
        }

        SECTION("operator>")
        {
            CHECK(sf::VideoMode{.size = {1, 0}} > sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 1});
            CHECK(sf::VideoMode{.size = {800, 800}, .bitsPerPixel = 48} >
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 24});
            CHECK(sf::VideoMode{.size = {600, 400}, .bitsPerPixel = 48} >
                  sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} >
                  sf::VideoMode{.size = {400, 400}, .bitsPerPixel = 48});
        }

        SECTION("operator<=")
        {
            CHECK(sf::VideoMode{.size = {800, 800}, .bitsPerPixel = 24} <=
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} <=
                  sf::VideoMode{.size = {600, 400}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 400}, .bitsPerPixel = 48} <=
                  sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} <= sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} <=
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }

        SECTION("operator>=")
        {
            CHECK(sf::VideoMode{.size = {1, 0}} >= sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 1});
            CHECK(sf::VideoMode{.size = {800, 800}, .bitsPerPixel = 48} >=
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 24});
            CHECK(sf::VideoMode{.size = {600, 400}, .bitsPerPixel = 48} >=
                  sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {400, 600}, .bitsPerPixel = 48} >=
                  sf::VideoMode{.size = {400, 400}, .bitsPerPixel = 48});
            CHECK(sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0} >= sf::VideoMode{.size = {0, 0}, .bitsPerPixel = 0});
            CHECK(sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64} >=
                  sf::VideoMode{.size = {1080, 1920}, .bitsPerPixel = 64});
        }
    }
}
