#include "SFML/Graphics/RenderStates.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"

#include <Doctest.hpp>

#include "GraphicsUtil.hpp"

TEST_CASE("[Graphics] sf::RenderStates")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::RenderStates));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::RenderStates)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::RenderStates));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::RenderStates, sf::RenderStates));

        STATIC_CHECK(sizeof(sf::RenderStates) <= 128); // TODO P0: this is big...
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::RenderStates renderStates{};
            CHECK(renderStates.blendMode == sf::BlendAlpha);
            CHECK(renderStates.stencilMode == sf::StencilMode{});
            CHECK(renderStates.transform == sf::Transform{});
            CHECK(renderStates.view == sf::View{});
            CHECK(renderStates.texture == nullptr);
            CHECK(renderStates.shader == nullptr);
        }
    }

    SECTION("Default constant")
    {
        CHECK(sf::RenderStates{}.blendMode == sf::BlendAlpha);
        CHECK(sf::RenderStates{}.stencilMode == sf::StencilMode{});
        CHECK(sf::RenderStates{}.transform == sf::Transform{});
        CHECK(sf::RenderStates{}.view == sf::View{});
        CHECK(sf::RenderStates{}.texture == nullptr);
        CHECK(sf::RenderStates{}.shader == nullptr);
    }
}
