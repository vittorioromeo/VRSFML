#include "SFML/Graphics/StencilMode.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::StencilMode")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::StencilMode));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::StencilMode));
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
            CHECK(sf::StencilMode{sf::StencilComparison::Equal,
                                  sf::StencilUpdateOperation::Replace,
                                  sf::StencilValue{1u},
                                  sf::StencilValue{0u},
                                  true} ==
                  sf::StencilMode{sf::StencilComparison::Equal,
                                  sf::StencilUpdateOperation::Replace,
                                  sf::StencilValue{1u},
                                  sf::StencilValue{0u},
                                  true});

            CHECK_FALSE(sf::StencilMode{} ==
                        sf::StencilMode{sf::StencilComparison::Equal,
                                        sf::StencilUpdateOperation::Replace,
                                        sf::StencilValue{1u},
                                        sf::StencilValue{0u},
                                        true});
            CHECK_FALSE(sf::StencilMode{sf::StencilComparison::Greater,
                                        sf::StencilUpdateOperation::Invert,
                                        sf::StencilValue{0u},
                                        sf::StencilValue{~0u},
                                        false} ==
                        sf::StencilMode{sf::StencilComparison::Equal,
                                        sf::StencilUpdateOperation::Replace,
                                        sf::StencilValue{1u},
                                        sf::StencilValue{0u},
                                        true});
        }

        SECTION("operator!=")
        {
            CHECK_FALSE(sf::StencilMode{} != sf::StencilMode{});
            CHECK_FALSE(sf::StencilMode{sf::StencilComparison::Equal,
                                        sf::StencilUpdateOperation::Replace,
                                        sf::StencilValue{1u},
                                        sf::StencilValue{0u},
                                        true} !=
                        sf::StencilMode{sf::StencilComparison::Equal,
                                        sf::StencilUpdateOperation::Replace,
                                        sf::StencilValue{1u},
                                        sf::StencilValue{0u},
                                        true});

            CHECK(sf::StencilMode{} !=
                  sf::StencilMode{sf::StencilComparison::Equal,
                                  sf::StencilUpdateOperation::Replace,
                                  sf::StencilValue{1u},
                                  sf::StencilValue{0u},
                                  true});
            CHECK(sf::StencilMode{sf::StencilComparison::Greater,
                                  sf::StencilUpdateOperation::Invert,
                                  sf::StencilValue{0u},
                                  sf::StencilValue{~0u},
                                  false} !=
                  sf::StencilMode{sf::StencilComparison::Equal,
                                  sf::StencilUpdateOperation::Replace,
                                  sf::StencilValue{1u},
                                  sf::StencilValue{0u},
                                  true});
        }
    }
}
