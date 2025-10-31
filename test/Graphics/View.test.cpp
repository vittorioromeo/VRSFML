#include "SFML/Graphics/View.hpp"

#include "SFML/System/Rect2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>


TEST_CASE("[Graphics] sf::View")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::View));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::View));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::View));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::View));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::View, sf::View));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::View view;
            CHECK(view.center == sf::Vec2f{500, 500});
            CHECK(view.size == sf::Vec2f{1000, 1000});
            CHECK(view.rotation == sf::Angle::Zero);
            CHECK(view.viewport == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.getTransform() == sf::Transform(0.002f, 0, -1, 0, -0.002f, 1));
            CHECK(view.getInverseTransform() == Approx(sf::Transform(500, 0, 500, 0, -500, 500)));
        }

        SECTION("Rectangle factory function")
        {
            const auto view = sf::View::fromRect({{10, 20}, {400, 600}});
            CHECK(view.center == sf::Vec2f{210, 320});
            CHECK(view.size == sf::Vec2f{400, 600});
            CHECK(view.rotation == sf::Angle::Zero);
            CHECK(view.viewport == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.getTransform() == Approx(sf::Transform(0.005f, 0, -1.05f, 0, -0.00333333f, 1.06667f)));
            CHECK(view.getInverseTransform() == Approx(sf::Transform(200, 0, 210, 0, -300, 320)));
        }

        SECTION("Center + size constructor")
        {
            const sf::View view({520, 960}, {1080, 1920});
            CHECK(view.center == sf::Vec2f{520, 960});
            CHECK(view.size == sf::Vec2f{1080, 1920});
            CHECK(view.rotation == sf::Angle::Zero);
            CHECK(view.viewport == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.scissor == sf::Rect2f({0, 0}, {1, 1}));
            CHECK(view.getTransform() == Approx(sf::Transform(0.00185185f, 0, -0.962963f, 0, -0.00104167f, 1)));
            CHECK(view.getInverseTransform() == Approx(sf::Transform(540, 0, 520, 0, -960, 960)));
        }
    }

    SECTION("Set/get center")
    {
        sf::View view;
        view.center = {3.14f, 4.2f};
        CHECK(view.center == sf::Vec2f(3.14f, 4.2f));
        CHECK(view.getTransform() == Approx(sf::Transform(0.002f, 0, -0.00628f, 0, -0.002f, 0.0084f)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(500, 0, 3.14f, 0, -500, 4.2f)));
    }

    SECTION("Set/get size")
    {
        sf::View view;
        view.size = {600, 900};
        CHECK(view.size == sf::Vec2f{600, 900});
        CHECK(view.getTransform() == Approx(sf::Transform(0.00333333f, 0, -1.66667f, 0, -0.00222222f, 1.11111f)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(300, 0, 500, 0, -450, 500)));
    }

    SECTION("Set/get rotation")
    {
        sf::View view;
        view.rotation = sf::degrees(-345);
        CHECK(view.rotation == Approx(sf::degrees(15)));
        CHECK(view.getTransform() ==
              Approx(sf::Transform(0.00193185f, 0.000517638f, -1.22474f, 0.000517638f, -0.00193185f, 0.707107f)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(482.963f, 129.41f, 500, 129.41f, -482.963f, 500)));
        view.rotation = sf::degrees(400);
        CHECK(view.rotation == Approx(sf::degrees(40)));
        CHECK(view.getTransform() ==
              Approx(sf::Transform(0.00153209f, 0.00128558f, -1.40883f, 0.00128558f, -0.00153209f, 0.123257f)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(383.022f, 321.394f, 500, 321.394f, -383.022f, 500)));
    }

    SECTION("Set/get viewport")
    {
        sf::View view;
        view.viewport = {{150, 250}, {500, 750}};
        CHECK(view.viewport == sf::Rect2f({150, 250}, {500, 750}));
        CHECK(view.getTransform() == Approx(sf::Transform(0.002f, 0, -1, 0, -0.002f, 1)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(500, 0, 500, 0, -500, 500)));
        CHECK(view.scissor == sf::Rect2f({0, 0}, {1, 1}));
    }

    SECTION("Set/get scissor")
    {
        sf::View view;
        view.scissor = sf::Rect2f{{0, 0}, {0.5f, 1}};
        CHECK(view.scissor == sf::Rect2f({0, 0}, {0.5, 1}));
        CHECK(view.viewport == sf::Rect2f({0, 0}, {1, 1}));
    }

    SECTION("rotate()")
    {
        sf::View view;
        view.rotation = sf::degrees(45);
        view.rotation += sf::degrees(-15);
        CHECK(view.rotation == Approx(sf::degrees(30)));
        CHECK(view.getTransform() ==
              Approx(sf::Transform(0.00173205f, 0.001f, -1.36603f, 0.001f, -0.00173205f, 0.366025f)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(433.013f, 250, 500, 250, -433.013f, 500)));
    }

    SECTION("zoom()")
    {
        sf::View view;
        view.size = {25, 25};
        view.size *= 4.f;
        CHECK(view.size == sf::Vec2f(100, 100));
        CHECK(view.getTransform() == Approx(sf::Transform(0.02f, 0, -10, 0, -0.02f, 10)));
        CHECK(view.getInverseTransform() == Approx(sf::Transform(50, 0, 500, 0, -50, 500)));
    }
}
