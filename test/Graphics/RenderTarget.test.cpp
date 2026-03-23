#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <SystemUtil.hpp>

namespace
{

class TestRenderTarget : public sf::RenderTarget
{
public:
    TestRenderTarget() = default;

private:
    [[nodiscard]] sf::Vec2u getSize() const override
    {
        return {640, 480};
    }
};

TEST_CASE("[Graphics] sf::RenderTarget")
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::RenderTarget));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::RenderTarget));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::RenderTarget));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::RenderTarget));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::RenderTarget));
    }

    SECTION("Construction")
    {
        const TestRenderTarget renderTarget;

        const auto defaultView = sf::View::fromScreenSize({640.f, 480.f});

        CHECK(defaultView.center == sf::Vec2f{320, 240});
        CHECK(defaultView.size == sf::Vec2f{640, 480});
        CHECK(defaultView.rotation == sf::Angle::Zero);
        CHECK(defaultView.viewport == sf::Rect2f({0, 0}, {1, 1}));
        CHECK(!renderTarget.isSrgb());
    }

    SECTION("Move assignment")
    {
        TestRenderTarget renderTarget0;
        TestRenderTarget renderTarget1;

        renderTarget1 = SFML_BASE_MOVE(renderTarget0);
    }

    SECTION("setActive()")
    {
        TestRenderTarget renderTarget;
        CHECK(renderTarget.setActive());
        CHECK(renderTarget.setActive(false));
        CHECK(renderTarget.setActive(true));
    }
}

} // namespace
