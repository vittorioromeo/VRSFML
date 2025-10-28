#include "SFML/System/Vec2.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/Math/Sqrt.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

using namespace sf::Literals;

TEMPLATE_TEST_CASE("[System] sf::Vec2", "", int, float)
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Vec2<TestType>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Vec2<TestType>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Vec2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Vec2<TestType>, sf::Vec2<TestType>));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr sf::Vec2<TestType> vec{};
            STATIC_CHECK(vec.x == 0);
            STATIC_CHECK(vec.y == 0);
        }

        SECTION("(x, y) coordinate constructor")
        {
            constexpr sf::Vec2<TestType> vec(1, 2);
            STATIC_CHECK(vec.x == 1);
            STATIC_CHECK(vec.y == 2);
        }

        SECTION("Length and angle constructor")
        {
            CHECK(sf::Vec2f::fromAngle(0, 0_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 45_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 90_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 135_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 180_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 270_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, 360_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, -90_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, -180_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, -270_deg) == sf::Vec2f{0, 0});
            CHECK(sf::Vec2f::fromAngle(0, -360_deg) == sf::Vec2f{0, 0});

            CHECK(sf::Vec2f::fromAngle(1, 0_deg) == sf::Vec2f{1, 0});
            CHECK(sf::Vec2f::fromAngle(1, 45_deg) ==
                  Approx(sf::Vec2f(sf::base::sqrt(2.f) / 2.f, sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(1, 90_deg) == Approx(sf::Vec2f{0, 1}));
            CHECK(sf::Vec2f::fromAngle(1, 135_deg) ==
                  Approx(sf::Vec2f(-sf::base::sqrt(2.f) / 2.f, sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(1, 180_deg) == Approx(sf::Vec2f(-1, 0)));
            CHECK(sf::Vec2f::fromAngle(1, 270_deg) == Approx(sf::Vec2f(0, -1)));
            CHECK(sf::Vec2f::fromAngle(1, 360_deg) == Approx(sf::Vec2f{1, 0}));
            CHECK(sf::Vec2f::fromAngle(1, -90_deg) == Approx(sf::Vec2f(0, -1)));
            CHECK(sf::Vec2f::fromAngle(1, -180_deg) == Approx(sf::Vec2f(-1, 0)));
            CHECK(sf::Vec2f::fromAngle(1, -270_deg) == Approx(sf::Vec2f{0, 1}));
            CHECK(sf::Vec2f::fromAngle(1, -360_deg) == Approx(sf::Vec2f{1, 0}));

            CHECK(sf::Vec2f::fromAngle(-1, 0_deg) == sf::Vec2f(-1, 0));
            CHECK(sf::Vec2f::fromAngle(-1, 45_deg) ==
                  Approx(sf::Vec2f(-sf::base::sqrt(2.f) / 2.f, -sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(-1, 90_deg) == Approx(sf::Vec2f(0, -1)));
            CHECK(sf::Vec2f::fromAngle(-1, 135_deg) ==
                  Approx(sf::Vec2f(sf::base::sqrt(2.f) / 2.f, -sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(-1, 180_deg) == Approx(sf::Vec2f{1, 0}));
            CHECK(sf::Vec2f::fromAngle(-1, 270_deg) == Approx(sf::Vec2f{0, 1}));
            CHECK(sf::Vec2f::fromAngle(-1, 360_deg) == Approx(sf::Vec2f(-1, 0)));
            CHECK(sf::Vec2f::fromAngle(-1, -90_deg) == Approx(sf::Vec2f{0, 1}));
            CHECK(sf::Vec2f::fromAngle(-1, -180_deg) == Approx(sf::Vec2f{1, 0}));
            CHECK(sf::Vec2f::fromAngle(-1, -270_deg) == Approx(sf::Vec2f(0, -1)));
            CHECK(sf::Vec2f::fromAngle(-1, -360_deg) == Approx(sf::Vec2f(-1, 0)));

            CHECK(sf::Vec2f::fromAngle(4.2f, 0_deg) == sf::Vec2f(4.2f, 0));
            CHECK(sf::Vec2f::fromAngle(4.2f, 45_deg) ==
                  Approx(sf::Vec2f(4.2f * sf::base::sqrt(2.f) / 2.f, 4.2f * sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, 90_deg) == Approx(sf::Vec2f(0, 4.2f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, 135_deg) ==
                  Approx(sf::Vec2f(-4.2f * sf::base::sqrt(2.f) / 2.f, 4.2f * sf::base::sqrt(2.f) / 2.f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, 180_deg) == Approx(sf::Vec2f(-4.2f, 0)));
            CHECK(sf::Vec2f::fromAngle(4.2f, 270_deg) == Approx(sf::Vec2f(0, -4.2f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, 360_deg) == Approx(sf::Vec2f(4.2f, 0)));
            CHECK(sf::Vec2f::fromAngle(4.2f, -90_deg) == Approx(sf::Vec2f(0, -4.2f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, -180_deg) == Approx(sf::Vec2f(-4.2f, 0)));
            CHECK(sf::Vec2f::fromAngle(4.2f, -270_deg) == Approx(sf::Vec2f(0, 4.2f)));
            CHECK(sf::Vec2f::fromAngle(4.2f, -360_deg) == Approx(sf::Vec2f(4.2f, 0)));
        }
    }

    SECTION("Unary operations")
    {
        SECTION("-vec")
        {
            constexpr sf::Vec2<TestType> vec(1, 2);
            constexpr sf::Vec2<TestType> negatedVec = -vec;

            STATIC_CHECK(negatedVec.x == -1);
            STATIC_CHECK(negatedVec.y == -2);
        }
    }

    SECTION("Arithmetic operations between two vecs")
    {
        sf::Vec2<TestType>           firstVec(2, 5);
        constexpr sf::Vec2<TestType> secondVec(8, 3);

        SECTION("vec += vec")
        {
            firstVec += secondVec;

            CHECK(firstVec.x == 10);
            CHECK(firstVec.y == 8);
        }

        SECTION("vec -= vec")
        {
            firstVec -= secondVec;

            CHECK(firstVec.x == -6);
            CHECK(firstVec.y == 2);
        }

        SECTION("vec + vec")
        {
            const sf::Vec2<TestType> result = firstVec + secondVec;

            CHECK(result.x == 10);
            CHECK(result.y == 8);
        }

        SECTION("vec - vec")
        {
            const sf::Vec2<TestType> result = firstVec - secondVec;

            CHECK(result.x == -6);
            CHECK(result.y == 2);
        }
    }

    SECTION("Arithmetic operations between vec and scalar value")
    {
        sf::Vec2<TestType> vec(26, 12);
        const TestType     scalar = 2;

        SECTION("vec * scalar")
        {
            const sf::Vec2<TestType> result = vec * scalar;

            CHECK(result.x == 52);
            CHECK(result.y == 24);
        }

        SECTION("scalar * vec")
        {
            const sf::Vec2<TestType> result = scalar * vec;

            CHECK(result.x == 52);
            CHECK(result.y == 24);
        }

        SECTION("vec *= scalar")
        {
            vec *= scalar;

            CHECK(vec.x == 52);
            CHECK(vec.y == 24);
        }

        SECTION("vec / scalar")
        {
            const sf::Vec2<TestType> result = vec / scalar;

            CHECK(result.x == 13);
            CHECK(result.y == 6);
        }

        SECTION("vec /= scalar")
        {
            vec /= scalar;

            CHECK(vec.x == 13);
            CHECK(vec.y == 6);
        }
    }

    SECTION("Comparison operations (two equal and one different vec)")
    {
        constexpr sf::Vec2<TestType> firstEqualVec(1, 5);
        constexpr sf::Vec2<TestType> secondEqualVec(1, 5);
        constexpr sf::Vec2<TestType> differentVec(6, 9);

        SECTION("vec == vec")
        {
            STATIC_CHECK(firstEqualVec == secondEqualVec);
            STATIC_CHECK_FALSE(firstEqualVec == differentVec);
        }

        SECTION("vec != vec")
        {
            STATIC_CHECK(firstEqualVec != differentVec);
            STATIC_CHECK_FALSE(firstEqualVec != secondEqualVec);
        }
    }

    SECTION("Structured bindings")
    {
        sf::Vec2<TestType> vec(1, 2); // NOLINT(misc-const-correctness)

        SECTION("destructure by value")
        {
            auto [x, y] = vec;

            CHECK(x == 1);
            CHECK(y == 2);

            STATIC_CHECK(SFML_BASE_IS_SAME(decltype(x), decltype(vec.x)));

            x = 3;

            CHECK(x == 3);
            CHECK(vec.x == 1);
        }

        SECTION("destructure by ref")
        {
            auto& [x, y] = vec;

            CHECK(x == 1);
            CHECK(y == 2);

            STATIC_CHECK(SFML_BASE_IS_SAME(decltype(x), decltype(vec.x)));

            x = 3;

            CHECK(x == 3);
            CHECK(vec.x == 3);
        }
    }

    SECTION("Length and normalization")
    {
        constexpr sf::Vec2f v(2.4f, 3.f);

        CHECK(v.length() == Approx(3.84187f));
        CHECK(v.lengthSquared() == Approx(14.7599650969f));
        CHECK(v.normalized() == Approx(sf::Vec2f(0.624695f, 0.780869f)));

        constexpr sf::Vec2f w(-0.7f, -2.2f);

        CHECK(w.length() == Approx(2.30868f));
        CHECK(w.lengthSquared() == Approx(5.3300033f));
        CHECK(w.normalized() == Approx(sf::Vec2f(-0.303204f, -0.952926f)));
    }

    SECTION("Rotations and angles")
    {
        constexpr sf::Vec2f v(2.4f, 3.f);

        CHECK(v.angle() == Approx(51.3402_deg));
        CHECK(sf::Vec2f{1.f, 0.f}.angleTo(v) == Approx(51.3402_deg));
        CHECK(sf::Vec2f{0.f, 1.f}.angleTo(v) == Approx(-38.6598_deg));

        constexpr sf::Vec2f w(-0.7f, -2.2f);

        CHECK(w.angle() == Approx(-107.65_deg));
        CHECK(sf::Vec2f{1.f, 0.f}.angleTo(w) == Approx(-107.65_deg));
        CHECK(sf::Vec2f{0.f, 1.f}.angleTo(w) == Approx(162.35_deg));

        CHECK(v.angleTo(w) == Approx(-158.9902_deg));
        CHECK(w.angleTo(v) == Approx(158.9902_deg));

        const float ratio = w.length() / v.length();
        CHECK(v.rotatedBy(-158.9902_deg) * ratio == Approx(w));
        CHECK(w.rotatedBy(158.9902_deg) / ratio == Approx(v));

        STATIC_CHECK(v.perpendicular() == sf::Vec2f(-3.f, 2.4f));
        STATIC_CHECK(v.perpendicular().perpendicular().perpendicular().perpendicular() == v);

        CHECK(v.rotatedBy(90_deg) == Approx(sf::Vec2f(-3.f, 2.4f)));
        CHECK(v.rotatedBy(27.14_deg) == Approx(sf::Vec2f(0.767248f, 3.76448f)));
        CHECK(v.rotatedBy(-36.11_deg) == Approx(sf::Vec2f(3.70694f, 1.00925f)));
    }

    SECTION("Products and quotients")
    {
        constexpr sf::Vec2f v(2.4f, 3.f);
        constexpr sf::Vec2f w(-0.7f, -2.2f);

        CHECK(v.dot(w) == Approx(-8.28f));
        CHECK(w.dot(v) == Approx(-8.28f));

        CHECK(v.cross(w) == Approx(-3.18f));
        CHECK(w.cross(v) == Approx(+3.18f));

        CHECK(v.componentWiseMul(w) == Approx(sf::Vec2f(-1.68f, -6.6f)));
        CHECK(w.componentWiseMul(v) == Approx(sf::Vec2f(-1.68f, -6.6f)));
        CHECK(v.componentWiseDiv(w) == Approx(sf::Vec2f(-3.428571f, -1.363636f)));
        CHECK(w.componentWiseDiv(v) == Approx(sf::Vec2f(-0.291666f, -0.733333f)));
    }

    SECTION("Projection")
    {
        constexpr sf::Vec2f v(2.4f, 3.f);
        constexpr sf::Vec2f w(-0.7f, -2.2f);

        CHECK(v.projectedOnto(w) == Approx(sf::Vec2f(1.087430f, 3.417636f)));
        CHECK(v.projectedOnto(w) == Approx(-1.55347f * w));

        CHECK(w.projectedOnto(v) == Approx(sf::Vec2f(-1.346342f, -1.682927f)));
        CHECK(w.projectedOnto(v) == Approx(-0.560976f * v));

        CHECK(v.projectedOnto(sf::Vec2f{1.f, 0.f}) == Approx(sf::Vec2f(2.4f, 0.f)));
        CHECK(v.projectedOnto(sf::Vec2f{0.f, 1.f}) == Approx(sf::Vec2f(0.f, 3.f)));
    }

    SECTION("Constexpr support")
    {
        constexpr sf::Vec2<TestType> v(1, 2);
        constexpr sf::Vec2<TestType> w(2, -6);

        STATIC_CHECK(v.x == 1);
        STATIC_CHECK(v.y == 2);
        STATIC_CHECK(v + w == sf::Vec2<TestType>(3, -4));

        STATIC_CHECK(v.lengthSquared() == 5);
        STATIC_CHECK(v.perpendicular() == sf::Vec2<TestType>(-2, 1));

        STATIC_CHECK(v.dot(w) == -10);
        STATIC_CHECK(v.cross(w) == -10);
        STATIC_CHECK(v.componentWiseMul(w) == sf::Vec2<TestType>(2, -12));
        STATIC_CHECK(w.componentWiseDiv(v) == sf::Vec2<TestType>(2, -3));
    }

    SECTION("Moved towards")
    {
        CHECK(sf::Vec2f{}.rotatedBy(-158.9902_deg) == sf::Vec2f{}.movedTowards(0.f, -158.9902_deg));

        constexpr sf::Vec2f v(2.4f, 3.f);
        CHECK(v + sf::Vec2f::fromAngle(10.f, -158.9902_deg) == v.movedTowards(10.f, -158.9902_deg));
    }

    SECTION("addX")
    {
        const sf::Vec2<TestType> vec(1, 2);

        const auto result = vec.addX(3);
        CHECK(result.x == 4);
        CHECK(result.y == 2);
    }

    SECTION("addY")
    {
        const sf::Vec2<TestType> vec(1, 2);

        const auto result = vec.addY(3);
        CHECK(result.x == 1);
        CHECK(result.y == 5);
    }

    SECTION("withX")
    {
        const sf::Vec2<TestType> vec(1, 2);

        const auto result = vec.withX(3);
        CHECK(result.x == 3);
        CHECK(result.y == 2);
    }

    SECTION("withY")
    {
        const sf::Vec2<TestType> vec(1, 2);

        const auto result = vec.withY(3);
        CHECK(result.x == 1);
        CHECK(result.y == 3);
    }

    SECTION("clampX")
    {
        const sf::Vec2<TestType> vec(100, 2);

        const auto result = vec.clampX(0, 50);
        CHECK(result.x == 50);
        CHECK(result.y == 2);
    }

    SECTION("clampY")
    {
        const sf::Vec2<TestType> vec(2, 100);

        const auto result = vec.clampY(0, 50);
        CHECK(result.x == 2);
        CHECK(result.y == 50);
    }
}
