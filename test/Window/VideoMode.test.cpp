#include "SFML/Window/VideoMode.hpp"

#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/Base/Algorithm.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Window] sf::VideoMode" * doctest::skip(skipDisplayTests))
{
    SECTION("Type traits")
    {
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
            CHECK(videoMode.size == sf::Vector2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 32);
        }

        SECTION("Width, height, bit depth constructor")
        {
            const sf::VideoMode videoMode{.size{800u, 600u}, .bitsPerPixel = 24u};
            CHECK(videoMode.size == sf::Vector2u{800, 600});
            CHECK(videoMode.bitsPerPixel == 24);
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
            CHECK(sf::VideoMode{{0, 0}, 0} == sf::VideoMode{{0, 0}, 0});
            CHECK(sf::VideoMode{{1080, 1920}, 64} == sf::VideoMode{{1080, 1920}, 64});
        }

        SECTION("operator!=")
        {
            CHECK(sf::VideoMode{{720, 720}} != sf::VideoMode{{720, 720}, 24});
            CHECK(sf::VideoMode{{1080, 1920}, 16} != sf::VideoMode{{400, 600}});
        }

        SECTION("operator<")
        {
            CHECK(sf::VideoMode{{800, 800}, 24} < sf::VideoMode{{1080, 1920}, 48});
            CHECK(sf::VideoMode{{400, 600}, 48} < sf::VideoMode{{600, 400}, 48});
            CHECK(sf::VideoMode{{400, 400}, 48} < sf::VideoMode{{400, 600}, 48});
        }

        SECTION("operator>")
        {
            CHECK(sf::VideoMode{{1, 0}} > sf::VideoMode{{0, 0}, 1});
            CHECK(sf::VideoMode{{800, 800}, 48} > sf::VideoMode{{1080, 1920}, 24});
            CHECK(sf::VideoMode{{600, 400}, 48} > sf::VideoMode{{400, 600}, 48});
            CHECK(sf::VideoMode{{400, 600}, 48} > sf::VideoMode{{400, 400}, 48});
        }

        SECTION("operator<=")
        {
            CHECK(sf::VideoMode{{800, 800}, 24} <= sf::VideoMode{{1080, 1920}, 48});
            CHECK(sf::VideoMode{{400, 600}, 48} <= sf::VideoMode{{600, 400}, 48});
            CHECK(sf::VideoMode{{400, 400}, 48} <= sf::VideoMode{{400, 600}, 48});
            CHECK(sf::VideoMode{{0, 0}, 0} <= sf::VideoMode{{0, 0}, 0});
            CHECK(sf::VideoMode{{1080, 1920}, 64} <= sf::VideoMode{{1080, 1920}, 64});
        }

        SECTION("operator>=")
        {
            CHECK(sf::VideoMode{{1, 0}} >= sf::VideoMode{{0, 0}, 1});
            CHECK(sf::VideoMode{{800, 800}, 48} >= sf::VideoMode{{1080, 1920}, 24});
            CHECK(sf::VideoMode{{600, 400}, 48} >= sf::VideoMode{{400, 600}, 48});
            CHECK(sf::VideoMode{{400, 600}, 48} >= sf::VideoMode{{400, 400}, 48});
            CHECK(sf::VideoMode{{0, 0}, 0} >= sf::VideoMode{{0, 0}, 0});
            CHECK(sf::VideoMode{{1080, 1920}, 64} >= sf::VideoMode{{1080, 1920}, 64});
        }
    }
}
