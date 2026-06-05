#include "GraphicsViewUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/View.hpp"

#include "Zancle/Geometry/Angle.hpp"
#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsStandardLayout.hpp"
#include "Zancle/Trait/IsTrivial.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyConstructible.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"
#include "Zancle/Trait/IsTriviallyMoveAssignable.hpp"
#include "Zancle/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::View")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::View));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::View));
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::View));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::View));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::View, za::View));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const za::View view{};
            CHECK(view.center == za::Vec2f{});
            CHECK(view.size == za::Vec2f{});
            CHECK(view.rotation == za::Angle::Zero);
            CHECK(view.viewport == za::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == za::Rect2f({0, 0}, {1, 1}));
        }

        SECTION("Rectangle factory function")
        {
            const auto view = za::View::fromRect({{10, 20}, {400, 600}});
            CHECK(view.center == za::Vec2f{210, 320});
            CHECK(view.size == za::Vec2f{400, 600});
            CHECK(view.rotation == za::Angle::Zero);
            CHECK(view.viewport == za::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == za::Rect2f({0, 0}, {1, 1}));
            CHECK(view.getTransform() == Approx(za::Transform(0.005f, 0, -1.05f, 0, -0.00333333f, 1.06667f)));
            CHECK(view.getInverseTransform() == Approx(za::Transform(200, 0, 210, 0, -300, 320)));
        }

        SECTION("Center + size constructor")
        {
            const za::View view{.center = {520, 960}, .size = {1080, 1920}};
            CHECK(view.center == za::Vec2f{520, 960});
            CHECK(view.size == za::Vec2f{1080, 1920});
            CHECK(view.rotation == za::Angle::Zero);
            CHECK(view.viewport == za::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == za::Rect2f({0, 0}, {1, 1}));
            CHECK(view.getTransform() == Approx(za::Transform(0.00185185f, 0, -0.962963f, 0, -0.00104167f, 1)));
            CHECK(view.getInverseTransform() == Approx(za::Transform(540, 0, 520, 0, -960, 960)));
        }
    }

    SECTION("Set/get center")
    {
        const za::View view{
            .center = {3.14f, 4.2f},
            .size   = {1000, 1000},
        };

        CHECK(view.center == za::Vec2f(3.14f, 4.2f));
        CHECK(view.getTransform() == Approx(za::Transform(0.002f, 0, -0.00628f, 0, -0.002f, 0.0084f)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(500, 0, 3.14f, 0, -500, 4.2f)));
    }

    SECTION("Set/get size")
    {
        const za::View view{
            .center = {500, 500},
            .size   = {600, 900},
        };

        CHECK(view.size == za::Vec2f{600, 900});
        CHECK(view.getTransform() == Approx(za::Transform(0.00333333f, 0, -1.66667f, 0, -0.00222222f, 1.11111f)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(300, 0, 500, 0, -450, 500)));
    }

    SECTION("Set/get rotation")
    {
        za::View view{
            .center   = {500, 500},
            .size     = {1000, 1000},
            .rotation = za::degrees(-345),
        };

        CHECK(view.rotation == Approx(za::degrees(15)));
        CHECK(view.getTransform() ==
              Approx(za::Transform(0.00193185f, 0.000517638f, -1.22474f, 0.000517638f, -0.00193185f, 0.707107f)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(482.963f, 129.41f, 500, 129.41f, -482.963f, 500)));

        view.rotation = za::degrees(400);
        CHECK(view.rotation == Approx(za::degrees(40)));
        CHECK(view.getTransform() ==
              Approx(za::Transform(0.00153209f, 0.00128558f, -1.40883f, 0.00128558f, -0.00153209f, 0.123257f)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(383.022f, 321.394f, 500, 321.394f, -383.022f, 500)));
    }

    SECTION("Set/get viewport")
    {
        const za::View view{
            .center   = {500, 500},
            .size     = {1000, 1000},
            .viewport = {{150, 250}, {500, 750}},
        };

        CHECK(view.viewport == za::Rect2f({150, 250}, {500, 750}));
        CHECK(view.getTransform() == Approx(za::Transform(0.002f, 0, -1, 0, -0.002f, 1)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(500, 0, 500, 0, -500, 500)));
        CHECK(view.scissor == za::Rect2f({0, 0}, {1, 1}));
    }

    SECTION("Set/get scissor")
    {
        za::View view;
        view.scissor = za::Rect2f{{0, 0}, {0.5f, 1}};
        CHECK(view.scissor == za::Rect2f({0, 0}, {0.5, 1}));
        CHECK(view.viewport == za::Rect2f({0, 0}, {1, 1}));
    }

    SECTION("rotate()")
    {
        za::View view{
            .center   = {500, 500},
            .size     = {1000, 1000},
            .rotation = za::degrees(45),
        };

        view.rotation += za::degrees(-15);

        CHECK(view.rotation == Approx(za::degrees(30)));
        CHECK(view.getTransform() ==
              Approx(za::Transform(0.00173205f, 0.001f, -1.36603f, 0.001f, -0.00173205f, 0.366025f)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(433.013f, 250, 500, 250, -433.013f, 500)));
    }

    SECTION("zoom()")
    {
        za::View view{
            .center = {500, 500},
            .size   = {25, 25},
        };

        view.size *= 4.f;

        CHECK(view.size == za::Vec2f(100, 100));
        CHECK(view.getTransform() == Approx(za::Transform(0.02f, 0, -10, 0, -0.02f, 10)));
        CHECK(view.getInverseTransform() == Approx(za::Transform(50, 0, 500, 0, -50, 500)));
    }

    SECTION("computePixelViewport")
    {
        CHECK(za::View{
                  .center   = {},
                  .size     = {},
                  .viewport = {{0, 0}, {1, 1}},
              }
                  .computePixelViewport({640, 480}) == za::Rect2i({0, 0}, {640, 480}));

        CHECK(za::View{
                  .center   = {},
                  .size     = {},
                  .viewport = {{1, 1}, {.5f, .25f}},
              }
                  .computePixelViewport({640, 480}) == za::Rect2i({640, 480}, {320, 120}));

        CHECK(za::View{
                  .center   = {},
                  .size     = {},
                  .viewport = {{.5f, .5f}, {.25f, .75f}},
              }
                  .computePixelViewport({640, 480}) == za::Rect2i({320, 240}, {160, 360}));
    }

    SECTION("computePixelScissor")
    {
        CHECK(za::View{
                  .center  = {},
                  .size    = {},
                  .scissor = {{0, 0}, {1, 1}},
              }
                  .computePixelScissor({640, 480}) == za::Rect2i({0, 0}, {640, 480}));

        CHECK(za::View{
                  .center  = {},
                  .size    = {},
                  .scissor = {{.5f, .5f}, {.25f, .25f}},
              }
                  .computePixelScissor({640, 480}) == za::Rect2i({320, 240}, {160, 120}));
    }

    SECTION("screenToWorld")
    {
        auto view = za::View::fromScreenSize({1000.f, 1000.f});
        auto size = za::Vec2f{640, 480};

        view.center += {5, 5};
        view.viewport = za::Rect2f({0, 0}, {.5f, 1});

        const auto [x1, y1] = view.screenToWorld({0, 0}, size);
        CHECK_THAT(x1, tst::Matchers::WithinRel(5, 1e-4));
        CHECK_THAT(y1, tst::Matchers::WithinRel(5, 1e-4));

        const auto [x2, y2] = view.screenToWorld({1, 1}, size);
        CHECK_THAT(x2, tst::Matchers::WithinRel(8.125, 1e-4));
        CHECK_THAT(y2, tst::Matchers::WithinRel(7.0833, 1e-4));

        const auto [x3, y3] = view.screenToWorld({320, 240}, size);
        CHECK_THAT(x3, tst::Matchers::WithinRel(1005, 1e-5));
        CHECK_THAT(y3, tst::Matchers::WithinRel(505, 1e-5));
    }

    SECTION("project")
    {
        auto view = za::View::fromScreenSize({1000.f, 1000.f});
        auto size = za::Vec2f{640, 480};

        view.center += {5, 5};
        view.viewport = za::Rect2f({.25f, 0}, {1, 1});

        const auto [x1, y1] = view.worldToScreen({0, 0}, size);
        CHECK_THAT(x1, tst::Matchers::WithinRel(156.8, 1e-2));
        CHECK_THAT(y1, tst::Matchers::WithinRel(-2.4, 1e-2));

        const auto [x2, y2] = view.worldToScreen({-500, 0}, size);
        CHECK_THAT(x2, tst::Matchers::WithinRel(-163.1, 1e-2));
        CHECK_THAT(y2, tst::Matchers::WithinRel(-2.4, 1e-2));

        const auto [x3, y3] = view.worldToScreen({0, -250}, size);
        CHECK_THAT(x3, tst::Matchers::WithinRel(156.8, 1e-2));
        CHECK_THAT(y3, tst::Matchers::WithinRel(-122.3, 1e-2));
    }
}
