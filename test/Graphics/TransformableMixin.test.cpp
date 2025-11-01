#include "SFML/Graphics/TransformableMixin.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>


struct TestTransformable : sf::TransformableMixin<TestTransformable>
{
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
};


TEST_CASE("[Graphics] sf::Transformable")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(TestTransformable));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(TestTransformable)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TestTransformable));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(TestTransformable, TestTransformable));
    }

    SECTION("Construction")
    {
        const TestTransformable transformable;
        CHECK(transformable.position == sf::Vec2f{0, 0});
        CHECK(transformable.rotation == sf::Angle::Zero);
        CHECK(transformable.scale == sf::Vec2f{1, 1});
        CHECK(transformable.origin == sf::Vec2f{0, 0});
        CHECK(transformable.getTransform() == sf::Transform());
        CHECK(transformable.getInverseTransform() == sf::Transform());
    }

    SECTION("Setters and getters")
    {
        TestTransformable transformable;

        transformable.position = {3, 4};
        CHECK(transformable.position == sf::Vec2f{3, 4});

        transformable.rotation = sf::degrees(3.14f);
        CHECK(transformable.rotation == Approx(sf::degrees(3.14f)));
        transformable.rotation = sf::degrees(540);
        CHECK(transformable.rotation == Approx(sf::degrees(180)));
        transformable.rotation = sf::degrees(-72);
        CHECK(transformable.rotation == Approx(sf::degrees(288)));

        transformable.scale = {5, 6};
        CHECK(transformable.scale == sf::Vec2f{5, 6});

        transformable.origin = {7, 8};
        CHECK(transformable.origin == sf::Vec2f{7, 8});

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

        transformable.getTransform().writeTo4x4Matrix(lhsMatrix);
        transform.writeTo4x4Matrix(rhsMatrix);

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
        transformable.getInverseTransform().writeTo4x4Matrix(lhsMatrix);
        inverseTransform.writeTo4x4Matrix(rhsMatrix);

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

    SECTION("Movement")
    {
        TestTransformable transformable;
        CHECK(transformable.position == sf::Vec2f{0, 0});
        transformable.position += {9, 10};
        CHECK(transformable.position == sf::Vec2f{9, 10});
        transformable.position += {-15, 2};
        CHECK(transformable.position == sf::Vec2f(-6, 12));
    }

    SECTION("Rotation")
    {
        TestTransformable transformable;
        CHECK(transformable.rotation == sf::Angle::Zero);
        transformable.rotation += sf::degrees(15);
        CHECK(transformable.rotation == Approx(sf::degrees(15)));
        transformable.rotation += sf::degrees(360);
        CHECK(transformable.rotation == Approx(sf::degrees(15)));
        transformable.rotation += sf::degrees(-25);
        CHECK(transformable.rotation == Approx(sf::degrees(350)));
        transformable.rotation += sf::degrees(-720);
        CHECK(transformable.rotation == Approx(sf::degrees(350)));
        transformable.rotation += sf::degrees(-370);
        CHECK(transformable.rotation == Approx(sf::degrees(340)));
    }

    SECTION("scaleBy()")
    {
        TestTransformable transformable;
        CHECK(transformable.scale == sf::Vec2f{1, 1});
        transformable.scaleBy({2, 3});
        CHECK(transformable.scale == sf::Vec2f{2, 3});
        transformable.scaleBy({2, 1});
        CHECK(transformable.scale == sf::Vec2f{4, 3});
        transformable.scaleBy({-1, -1});
        CHECK(transformable.scale == sf::Vec2f(-4, -3));
    }
}
