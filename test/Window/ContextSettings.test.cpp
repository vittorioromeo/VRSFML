#include "Tst/Tst.hpp"

#include "Zancle/Window/ContextSettings.hpp"

#include "Zancle/Trait/IsTriviallyCopyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyConstructible.hpp"
#include "Zancle/Trait/IsTriviallyMoveAssignable.hpp"
#include "Zancle/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Window] za::ContextSettings")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::ContextSettings));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::ContextSettings));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::ContextSettings));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::ContextSettings));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr za::ContextSettings contextSettings;
            STATIC_CHECK(contextSettings.depthBits == za::ContextSettings::defaultDepthBits);
            STATIC_CHECK(contextSettings.stencilBits == za::ContextSettings::defaultStencilBits);
            STATIC_CHECK(contextSettings.majorVersion == za::ContextSettings::defaultMajorVersion);
            STATIC_CHECK(contextSettings.minorVersion == za::ContextSettings::defaultMinorVersion);
            STATIC_CHECK(contextSettings.attributeFlags == za::ContextSettings::defaultAttributeFlags);
        }

        SECTION("Aggregate initialization -- Everything")
        {
            constexpr za::ContextSettings contextSettings{.depthBits      = 1,
                                                          .stencilBits    = 1,
                                                          .majorVersion   = 3,
                                                          .minorVersion   = 5,
                                                          .attributeFlags = za::ContextSettings::Attribute::Core};
            STATIC_CHECK(contextSettings.depthBits == 1);
            STATIC_CHECK(contextSettings.stencilBits == 1);
            STATIC_CHECK(contextSettings.majorVersion == 3);
            STATIC_CHECK(contextSettings.minorVersion == 5);
            STATIC_CHECK(contextSettings.attributeFlags == za::ContextSettings::Attribute::Core);
        }
    }
}
