#include "Zancle/Graphics/Sprite.hpp"

#include "Zancle/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Graphics/Texture.hpp"

#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[Graphics] za::Sprite" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = za::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::Sprite));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::Sprite));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Sprite));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Sprite));

        STATIC_CHECK(!ZB_IS_TRIVIAL(za::Sprite)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::Sprite));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::Sprite));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::Sprite));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::Sprite));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::Sprite, za::Sprite));
    }

    const auto texture = za::Texture::create({64, 64}).value();

    SECTION("Construction")
    {
        SECTION("Rectangle constructor")
        {
            const za::Sprite sprite{.textureRect = {{0.f, 0.f}, {40.f, 60.f}}};
            CHECK(sprite.textureRect == za::Rect2f({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.color == za::Color::White);
            CHECK(sprite.getLocalBounds() == za::Rect2f({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.getGlobalBounds() == za::Rect2f({0.f, 0.f}, {40.f, 60.f}));
        }

        SECTION("Negative-size texture rectangle")
        {
            const za::Sprite sprite{.textureRect = {{0.f, 0.f}, {-40.f, -60.f}}};
            CHECK(sprite.textureRect == za::Rect2f({0.f, 0.f}, {-40.f, -60.f}));
            CHECK(sprite.color == za::Color::White);
            CHECK(sprite.getLocalBounds() == za::Rect2f({0.f, 0.f}, {40.f, 60.f}));
            CHECK(sprite.getGlobalBounds() == za::Rect2f({0.f, 0.f}, {40.f, 60.f}));
        }
    }

    SECTION("Set/get texture rect")
    {
        za::Sprite sprite{.textureRect = {}};
        sprite.textureRect = {{1, 2}, {3, 4}};
        CHECK(sprite.textureRect == za::Rect2f({1, 2}, {3, 4}));
    }

    SECTION("Set/get color")
    {
        za::Sprite sprite{.textureRect = {}};
        sprite.color = za::Color::Red;
        CHECK(sprite.color == za::Color::Red);
    }
}
