#include "SFML/Graphics/RenderStates.hpp"

#include "SFML/Graphics/BlendMode.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::RenderStates")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::RenderStates));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::RenderStates)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::RenderStates, sf::RenderStates));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::RenderStates renderStates{};
            CHECK(renderStates.blendMode == sf::BlendAlpha);
            CHECK(renderStates.stencilMode == sf::StencilMode{});
            CHECK(renderStates.transform == sf::Transform());
            CHECK(renderStates.coordinateType == sf::CoordinateType::Pixels);
            CHECK(renderStates.texture == nullptr);
            CHECK(renderStates.shader == nullptr);
        }
    }

    SECTION("Default constant")
    {
        CHECK(sf::RenderStates::Default.blendMode == sf::BlendAlpha);
        CHECK(sf::RenderStates::Default.stencilMode == sf::StencilMode{});
        CHECK(sf::RenderStates::Default.transform == sf::Transform());
        CHECK(sf::RenderStates::Default.coordinateType == sf::CoordinateType::Pixels);
        CHECK(sf::RenderStates::Default.texture == nullptr);
        CHECK(sf::RenderStates::Default.shader == nullptr);
    }
}
