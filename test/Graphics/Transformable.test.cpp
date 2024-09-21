#include "SFML/Graphics/Transformable.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::Transformable")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Transformable));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Transformable)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Transformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Transformable, sf::Transformable));
    }

    SECTION("Construction")
    {
        const sf::Transformable transformable;
        CHECK(transformable.position == sf::Vector2f{0, 0});
        CHECK(transformable.rotation == sf::Angle::Zero);
        CHECK(transformable.scale == sf::Vector2f{1, 1});
        CHECK(transformable.origin == sf::Vector2f{0, 0});
        CHECK(transformable.getTransform() == sf::Transform());
        CHECK(transformable.getInverseTransform() == sf::Transform());
    }

    SECTION("Setters and getters")
    {
        sf::Transformable transformable;

        transformable.position = {3, 4};
        CHECK(transformable.position == sf::Vector2f{3, 4});

        transformable.rotation = sf::degrees(3.14f);
        CHECK(transformable.rotation == Approx(sf::degrees(3.14f)));
        transformable.rotation = sf::degrees(540).wrapUnsigned(); // TODO P0: make wrappable angle
        CHECK(transformable.rotation == Approx(sf::degrees(180)));
        transformable.rotation = sf::degrees(-72).wrapUnsigned(); // TODO P0: make wrappable angle
        CHECK(transformable.rotation == Approx(sf::degrees(288)));

        transformable.scale = {5, 6};
        CHECK(transformable.scale == sf::Vector2f{5, 6});

        transformable.origin = {7, 8};
        CHECK(transformable.origin == sf::Vector2f{7, 8});

        sf::Transform transform;
        transform.translate(transformable.position - transformable.origin);
        transform.rotate(transformable.rotation, transformable.origin);
        transform.scaleBy(transformable.scale, transformable.origin);

        // clang-format off
        float lhsMatrix[]{{},  {},  0.f, 0.f,
                          {},  {},  0.f, 0.f,
                          0.f, 0.f, 1.f, 0.f,
                          {},  {},  0.f, 1.f};

        float rhsMatrix[]{{},  {},  0.f, 0.f,
                          {},  {},  0.f, 0.f,
                          0.f, 0.f, 1.f, 0.f,
                          {},  {},  0.f, 1.f};
        // clang-format on

        transformable.getTransform().getMatrix(lhsMatrix);
        transform.getMatrix(rhsMatrix);

        CHECK(lhsMatrix[0] == Approx(rhsMatrix[0]));
        CHECK(lhsMatrix[1] == Approx(rhsMatrix[1]));
        CHECK(lhsMatrix[2] == Approx(rhsMatrix[2]));
        CHECK(lhsMatrix[3] == Approx(rhsMatrix[3]));
        CHECK(lhsMatrix[4] == Approx(rhsMatrix[4]));
        CHECK(lhsMatrix[5] == Approx(rhsMatrix[5]));
        CHECK(lhsMatrix[6] == Approx(rhsMatrix[6]));
        CHECK(lhsMatrix[7] == Approx(rhsMatrix[7]));
        CHECK(lhsMatrix[8] == Approx(rhsMatrix[8]));
        CHECK(lhsMatrix[9] == Approx(rhsMatrix[9]));
        CHECK(lhsMatrix[10] == Approx(rhsMatrix[10]));
        CHECK(lhsMatrix[11] == Approx(rhsMatrix[11]));
        CHECK(lhsMatrix[12] == Approx(rhsMatrix[12]));
        CHECK(lhsMatrix[13] == Approx(rhsMatrix[13]));
        CHECK(lhsMatrix[14] == Approx(rhsMatrix[14]));
        CHECK(lhsMatrix[15] == Approx(rhsMatrix[15]));

        const sf::Transform inverseTransform = transform.getInverse();
        transformable.getInverseTransform().getMatrix(lhsMatrix);
        inverseTransform.getMatrix(rhsMatrix);

        CHECK(lhsMatrix[0] == Approx(rhsMatrix[0]));
        CHECK(lhsMatrix[1] == Approx(rhsMatrix[1]));
        CHECK(lhsMatrix[2] == Approx(rhsMatrix[2]));
        CHECK(lhsMatrix[3] == Approx(rhsMatrix[3]));
        CHECK(lhsMatrix[4] == Approx(rhsMatrix[4]));
        CHECK(lhsMatrix[5] == Approx(rhsMatrix[5]));
        CHECK(lhsMatrix[6] == Approx(rhsMatrix[6]));
        CHECK(lhsMatrix[7] == Approx(rhsMatrix[7]));
        CHECK(lhsMatrix[8] == Approx(rhsMatrix[8]));
        CHECK(lhsMatrix[9] == Approx(rhsMatrix[9]));
        CHECK(lhsMatrix[10] == Approx(rhsMatrix[10]));
        CHECK(lhsMatrix[11] == Approx(rhsMatrix[11]));
        CHECK(lhsMatrix[12] == Approx(rhsMatrix[12]));
        CHECK(lhsMatrix[13] == Approx(rhsMatrix[13]));
        CHECK(lhsMatrix[14] == Approx(rhsMatrix[14]));
        CHECK(lhsMatrix[15] == Approx(rhsMatrix[15]));
    }

    SECTION("move()")
    {
        sf::Transformable transformable;
        CHECK(transformable.position == sf::Vector2f{0, 0});
        transformable.position += {9, 10};
        CHECK(transformable.position == sf::Vector2f{9, 10});
        transformable.position += {-15, 2};
        CHECK(transformable.position == sf::Vector2f(-6, 12));
    }

    SECTION("rotate()")
    {
        sf::Transformable transformable;
        CHECK(transformable.rotation == sf::Angle::Zero);
        transformable.rotate(sf::degrees(15));
        CHECK(transformable.rotation == Approx(sf::degrees(15)));
        transformable.rotate(sf::degrees(360));
        CHECK(transformable.rotation == Approx(sf::degrees(15)));
        transformable.rotate(sf::degrees(-25));
        CHECK(transformable.rotation == Approx(sf::degrees(350)));
        transformable.rotate(sf::degrees(-720));
        CHECK(transformable.rotation == Approx(sf::degrees(350)));
        transformable.rotate(sf::degrees(-370));
        CHECK(transformable.rotation == Approx(sf::degrees(340)));
    }

    SECTION("scale()")
    {
        sf::Transformable transformable;
        CHECK(transformable.scale == sf::Vector2f{1, 1});
        transformable.scaleBy({2, 3});
        CHECK(transformable.scale == sf::Vector2f{2, 3});
        transformable.scaleBy({2, 1});
        CHECK(transformable.scale == sf::Vector2f{4, 3});
        transformable.scaleBy({-1, -1});
        CHECK(transformable.scale == sf::Vector2f(-4, -3));
    }
}
