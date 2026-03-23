#include "SFML/Window/ContextSettings.hpp"

#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"

#include <Doctest.hpp>


TEST_CASE("[Window] sf::ContextSettings")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::ContextSettings));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::ContextSettings));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr sf::ContextSettings contextSettings;
            STATIC_CHECK(contextSettings.depthBits == sf::ContextSettings::defaultDepthBits);
            STATIC_CHECK(contextSettings.stencilBits == sf::ContextSettings::defaultStencilBits);
            STATIC_CHECK(contextSettings.majorVersion == sf::ContextSettings::defaultMajorVersion);
            STATIC_CHECK(contextSettings.minorVersion == sf::ContextSettings::defaultMinorVersion);
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::defaultAttributeFlags);
        }

        SECTION("Aggregate initialization -- Everything")
        {
            constexpr sf::ContextSettings contextSettings{1, 1, 3, 5, sf::ContextSettings::Attribute::Core};
            STATIC_CHECK(contextSettings.depthBits == 1);
            STATIC_CHECK(contextSettings.stencilBits == 1);
            STATIC_CHECK(contextSettings.majorVersion == 3);
            STATIC_CHECK(contextSettings.minorVersion == 5);
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::Attribute::Core);
        }
    }
}
