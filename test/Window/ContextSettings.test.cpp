#include "SFML/Window/ContextSettings.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

TEST_CASE("[Window] sf::ContextSettings")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::ContextSettings));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr sf::ContextSettings contextSettings;
            STATIC_CHECK(contextSettings.depthBits == 0);
            STATIC_CHECK(contextSettings.stencilBits == 0);
            STATIC_CHECK(contextSettings.antialiasingLevel == 0);
            STATIC_CHECK(contextSettings.majorVersion == sf::ContextSettings::defaultMajorVersion);
            STATIC_CHECK(contextSettings.minorVersion == sf::ContextSettings::defaultMinorVersion);
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::defaultAttributeFlags);
            STATIC_CHECK(contextSettings.sRgbCapable == false);
        }

        SECTION("Aggregate initialization -- Everything")
        {
            constexpr sf::ContextSettings contextSettings{1, 1, 2, 3, 5, sf::ContextSettings::Attribute::Core, true};
            STATIC_CHECK(contextSettings.depthBits == 1);
            STATIC_CHECK(contextSettings.stencilBits == 1);
            STATIC_CHECK(contextSettings.antialiasingLevel == 2);
            STATIC_CHECK(contextSettings.majorVersion == 3);
            STATIC_CHECK(contextSettings.minorVersion == 5);
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::Attribute::Core);
            STATIC_CHECK(contextSettings.sRgbCapable == true);
        }
    }
}
