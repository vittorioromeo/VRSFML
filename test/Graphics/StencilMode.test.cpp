#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/StencilMode.hpp"

#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsConvertible.hpp"
#include "Zancle/Trait/IsDefaultConstructible.hpp"
#include "Zancle/Trait/IsStandardLayout.hpp"
#include "Zancle/Trait/IsTrivial.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyConstructible.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"
#include "Zancle/Trait/IsTriviallyMoveAssignable.hpp"
#include "Zancle/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::StencilMode")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::StencilValue));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::StencilValue));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::StencilValue));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::StencilValue));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::StencilValue));
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::StencilValue));
        STATIC_CHECK(!ZA_IS_AGGREGATE(za::StencilValue)); // due to constructor
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::StencilValue));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::StencilValue));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::StencilValue, za::StencilValue));

        STATIC_CHECK(sizeof(za::StencilValue) <= sizeof(unsigned int));

        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::StencilMode));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::StencilMode));
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::StencilMode));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::StencilMode));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::StencilMode, za::StencilMode));

        STATIC_CHECK(sizeof(za::StencilMode) <= sizeof(unsigned int) * 4);
    }

    SECTION("Construction")
    {
        const za::StencilMode stencilMode;
        CHECK(stencilMode.stencilComparison == za::StencilComparison::Always);
        CHECK(stencilMode.stencilUpdateOperation == za::StencilUpdateOperation::Keep);
        CHECK(stencilMode.stencilReference.value == 0u);
        CHECK(stencilMode.stencilMask.value == ~0u);
        CHECK(stencilMode.stencilOnly == false);
    }

    SECTION("Stencil value type traits")
    {
        STATIC_CHECK(!ZA_IS_DEFAULT_CONSTRUCTIBLE(za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(bool, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(char, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(unsigned char, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(short, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(unsigned short, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(int, za::StencilValue));
        STATIC_CHECK(!ZA_IS_CONVERTIBLE(unsigned int, za::StencilValue));
    }

    SECTION("Stencil value construction")
    {
        const za::StencilValue stencilValue{0u};
        CHECK(stencilValue.value == 0u);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(za::StencilMode{} == za::StencilMode{});
            CHECK(za::StencilMode{
                      .stencilComparison      = za::StencilComparison::Equal,
                      .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                      .stencilOnly            = true,
                      .stencilReference       = za::StencilValue{1u},
                      .stencilMask            = za::StencilValue{0u},
                  } == za::StencilMode{
                           .stencilComparison      = za::StencilComparison::Equal,
                           .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                           .stencilOnly            = true,
                           .stencilReference       = za::StencilValue{1u},
                           .stencilMask            = za::StencilValue{0u},
                       });

            CHECK_FALSE(za::StencilMode{} ==
                        za::StencilMode{
                            .stencilComparison      = za::StencilComparison::Equal,
                            .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                            .stencilOnly            = true,
                            .stencilReference       = za::StencilValue{1u},
                            .stencilMask            = za::StencilValue{0u},
                        });
            CHECK_FALSE(
                za::StencilMode{
                    .stencilComparison      = za::StencilComparison::Greater,
                    .stencilUpdateOperation = za::StencilUpdateOperation::Invert,
                    .stencilOnly            = false,
                    .stencilReference       = za::StencilValue{0u},
                    .stencilMask            = za::StencilValue{~0u},
                } == za::StencilMode{
                         .stencilComparison      = za::StencilComparison::Equal,
                         .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                         .stencilOnly            = true,
                         .stencilReference       = za::StencilValue{1u},
                         .stencilMask            = za::StencilValue{0u},
                     });
        }

        SECTION("operator!=")
        {
            CHECK_FALSE(za::StencilMode{} != za::StencilMode{});
            CHECK_FALSE(
                za::StencilMode{
                    .stencilComparison      = za::StencilComparison::Equal,
                    .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                    .stencilOnly            = true,
                    .stencilReference       = za::StencilValue{1u},
                    .stencilMask            = za::StencilValue{0u},
                } != za::StencilMode{
                         .stencilComparison      = za::StencilComparison::Equal,
                         .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                         .stencilOnly            = true,
                         .stencilReference       = za::StencilValue{1u},
                         .stencilMask            = za::StencilValue{0u},
                     });

            CHECK(za::StencilMode{} !=
                  za::StencilMode{
                      .stencilComparison      = za::StencilComparison::Equal,
                      .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                      .stencilOnly            = true,
                      .stencilReference       = za::StencilValue{1u},
                      .stencilMask            = za::StencilValue{0u},
                  });
            CHECK(za::StencilMode{
                      .stencilComparison      = za::StencilComparison::Greater,
                      .stencilUpdateOperation = za::StencilUpdateOperation::Invert,
                      .stencilOnly            = false,
                      .stencilReference       = za::StencilValue{0u},
                      .stencilMask            = za::StencilValue{~0u},
                  } != za::StencilMode{
                           .stencilComparison      = za::StencilComparison::Equal,
                           .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                           .stencilOnly            = true,
                           .stencilReference       = za::StencilValue{1u},
                           .stencilMask            = za::StencilValue{0u},
                       });
        }
    }
}
