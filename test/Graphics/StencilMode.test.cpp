#include "SFML/Graphics/StencilMode.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>


TEST_CASE("[Graphics] sf::StencilMode")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::StencilValue));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::StencilValue)); // due to constructor
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::StencilValue));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::StencilValue, sf::StencilValue));

        STATIC_CHECK(sizeof(sf::StencilValue) <= sizeof(unsigned int));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::StencilMode));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::StencilMode, sf::StencilMode));

        STATIC_CHECK(sizeof(sf::StencilMode) <= sizeof(unsigned int) * 4);
    }

    SECTION("Construction")
    {
        const sf::StencilMode stencilMode;
        CHECK(stencilMode.stencilComparison == sf::StencilComparison::Always);
        CHECK(stencilMode.stencilUpdateOperation == sf::StencilUpdateOperation::Keep);
        CHECK(stencilMode.stencilReference.value == 0u);
        CHECK(stencilMode.stencilMask.value == ~0u);
        CHECK(stencilMode.stencilOnly == false);
    }

    SECTION("Stencil value type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(bool, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(char, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(unsigned char, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(short, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(unsigned short, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(int, sf::StencilValue));
        STATIC_CHECK(!SFML_BASE_IS_CONVERTIBLE(unsigned int, sf::StencilValue));
    }

    SECTION("Stencil value construction")
    {
        const sf::StencilValue stencilValue{0u};
        CHECK(stencilValue.value == 0u);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::StencilMode{} == sf::StencilMode{});
            CHECK(sf::StencilMode{
                      .stencilComparison      = sf::StencilComparison::Equal,
                      .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                      .stencilOnly            = true,
                      .stencilReference       = sf::StencilValue{1u},
                      .stencilMask            = sf::StencilValue{0u},
                  } == sf::StencilMode{
                           .stencilComparison      = sf::StencilComparison::Equal,
                           .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                           .stencilOnly            = true,
                           .stencilReference       = sf::StencilValue{1u},
                           .stencilMask            = sf::StencilValue{0u},
                       });

            CHECK_FALSE(sf::StencilMode{} ==
                        sf::StencilMode{
                            .stencilComparison      = sf::StencilComparison::Equal,
                            .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                            .stencilOnly            = true,
                            .stencilReference       = sf::StencilValue{1u},
                            .stencilMask            = sf::StencilValue{0u},
                        });
            CHECK_FALSE(
                sf::StencilMode{
                    .stencilComparison      = sf::StencilComparison::Greater,
                    .stencilUpdateOperation = sf::StencilUpdateOperation::Invert,
                    .stencilOnly            = false,
                    .stencilReference       = sf::StencilValue{0u},
                    .stencilMask            = sf::StencilValue{~0u},
                } == sf::StencilMode{
                         .stencilComparison      = sf::StencilComparison::Equal,
                         .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                         .stencilOnly            = true,
                         .stencilReference       = sf::StencilValue{1u},
                         .stencilMask            = sf::StencilValue{0u},
                     });
        }

        SECTION("operator!=")
        {
            CHECK_FALSE(sf::StencilMode{} != sf::StencilMode{});
            CHECK_FALSE(
                sf::StencilMode{
                    .stencilComparison      = sf::StencilComparison::Equal,
                    .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                    .stencilOnly            = true,
                    .stencilReference       = sf::StencilValue{1u},
                    .stencilMask            = sf::StencilValue{0u},
                } != sf::StencilMode{
                         .stencilComparison      = sf::StencilComparison::Equal,
                         .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                         .stencilOnly            = true,
                         .stencilReference       = sf::StencilValue{1u},
                         .stencilMask            = sf::StencilValue{0u},
                     });

            CHECK(sf::StencilMode{} !=
                  sf::StencilMode{
                      .stencilComparison      = sf::StencilComparison::Equal,
                      .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                      .stencilOnly            = true,
                      .stencilReference       = sf::StencilValue{1u},
                      .stencilMask            = sf::StencilValue{0u},
                  });
            CHECK(sf::StencilMode{
                      .stencilComparison      = sf::StencilComparison::Greater,
                      .stencilUpdateOperation = sf::StencilUpdateOperation::Invert,
                      .stencilOnly            = false,
                      .stencilReference       = sf::StencilValue{0u},
                      .stencilMask            = sf::StencilValue{~0u},
                  } != sf::StencilMode{
                           .stencilComparison      = sf::StencilComparison::Equal,
                           .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                           .stencilOnly            = true,
                           .stencilReference       = sf::StencilValue{1u},
                           .stencilMask            = sf::StencilValue{0u},
                       });
        }
    }
}
