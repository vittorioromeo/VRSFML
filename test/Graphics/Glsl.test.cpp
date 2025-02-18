#include "SFML/Graphics/Glsl.hpp"

// Other 1st party headers
#include "SFML/Graphics/Transform.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Graphics] sf::Glsl")
{
    SECTION("Vec2")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Vec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Vec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Vec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Vec2));
        }

        constexpr sf::Glsl::Vec2 vec{};
        STATIC_CHECK(vec.x == 0);
        STATIC_CHECK(vec.y == 0);
    }

    SECTION("Ivec2")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Ivec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Ivec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Ivec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Ivec2));
        }

        constexpr sf::Glsl::Ivec2 vec{};
        STATIC_CHECK(vec.x == 0);
        STATIC_CHECK(vec.y == 0);
    }

    SECTION("Bvec2")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Bvec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Bvec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Bvec2));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Bvec2));
        }

        constexpr sf::Glsl::Bvec2 vec{};
        STATIC_CHECK(vec.x == false);
        STATIC_CHECK(vec.y == false);
    }

    SECTION("Vec3")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Vec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Vec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Vec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Vec3));
        }

        constexpr sf::Glsl::Vec3 vec{};
        STATIC_CHECK(vec.x == 0);
        STATIC_CHECK(vec.y == 0);
        STATIC_CHECK(vec.z == 0);
    }

    SECTION("Ivec3")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Ivec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Ivec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Ivec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Ivec3));
        }

        constexpr sf::Glsl::Ivec3 vec{};
        STATIC_CHECK(vec.x == 0);
        STATIC_CHECK(vec.y == 0);
        STATIC_CHECK(vec.z == 0);
    }

    SECTION("Bvec3")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Bvec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Bvec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Bvec3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Bvec3));
        }

        constexpr sf::Glsl::Bvec3 vec{};
        STATIC_CHECK(vec.x == false);
        STATIC_CHECK(vec.y == false);
        STATIC_CHECK(vec.z == false);
    }

    SECTION("Vec4")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Vec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Vec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Vec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Vec4));
        }

        SECTION("Default constructor")
        {
            constexpr sf::Glsl::Vec4 vec{};
            STATIC_CHECK(vec.x == 0);
            STATIC_CHECK(vec.y == 0);
            STATIC_CHECK(vec.z == 0);
            STATIC_CHECK(vec.w == 0);
        }

        SECTION("Verbose constructor")
        {
            constexpr sf::Glsl::Vec4 vec(1, 2, 3, 4);
            STATIC_CHECK(vec.x == 1);
            STATIC_CHECK(vec.y == 2);
            STATIC_CHECK(vec.z == 3);
            STATIC_CHECK(vec.w == 4);
        }

        SECTION("Color constructor")
        {
            constexpr sf::Glsl::Vec4 vec = sf::Color(0, 128, 192, 255);
            STATIC_CHECK(vec.x == 0.f);
            STATIC_CHECK(vec.w == 1.f);
            CHECK(vec.y == Approx(128 / 255.f));
            CHECK(vec.z == Approx(192 / 255.f));
        }
    }

    SECTION("Ivec4")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Ivec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Ivec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Ivec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Ivec4));
        }

        SECTION("Default constructor")
        {
            constexpr sf::Glsl::Ivec4 vec{};
            STATIC_CHECK(vec.x == 0);
            STATIC_CHECK(vec.y == 0);
            STATIC_CHECK(vec.z == 0);
            STATIC_CHECK(vec.w == 0);
        }

        SECTION("Verbose constructor")
        {
            constexpr sf::Glsl::Ivec4 vec(1, 2, 3, 4);
            STATIC_CHECK(vec.x == 1);
            STATIC_CHECK(vec.y == 2);
            STATIC_CHECK(vec.z == 3);
            STATIC_CHECK(vec.w == 4);
        }

        SECTION("Color constructor")
        {
            constexpr sf::Glsl::Ivec4 vec = sf::Color(0, 128, 192, 255);
            STATIC_CHECK(vec.x == 0);
            STATIC_CHECK(vec.y == 128);
            STATIC_CHECK(vec.z == 192);
            STATIC_CHECK(vec.w == 255);
        }
    }

    SECTION("Bvec4")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Bvec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Bvec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Bvec4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Bvec4));
        }

        SECTION("Default constructor")
        {
            constexpr sf::Glsl::Bvec4 vec{};
            STATIC_CHECK(vec.x == false);
            STATIC_CHECK(vec.y == false);
            STATIC_CHECK(vec.z == false);
            STATIC_CHECK(vec.w == false);
        }

        SECTION("Verbose constructor")
        {
            constexpr sf::Glsl::Bvec4 vec(false, true, true, false);
            STATIC_CHECK(vec.x == false);
            STATIC_CHECK(vec.y == true);
            STATIC_CHECK(vec.z == true);
            STATIC_CHECK(vec.w == false);
        }
    }

    SECTION("Mat3")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Mat3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Mat3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Mat3));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Mat3));
        }

        SECTION("Array constructor")
        {
            static constexpr float data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
            const sf::Glsl::Mat3   mat(data);
            CHECK(mat.array[0] == 1);
            CHECK(mat.array[1] == 2);
            CHECK(mat.array[2] == 3);
            CHECK(mat.array[3] == 4);
            CHECK(mat.array[4] == 5);
            CHECK(mat.array[5] == 6);
            CHECK(mat.array[6] == 7);
            CHECK(mat.array[7] == 8);
            CHECK(mat.array[8] == 9);
        }

        SECTION("Transform constructor")
        {
            constexpr sf::Transform transform(10, 11, 12, 13, 14, 15);
            const sf::Glsl::Mat3    mat = transform;
            CHECK(mat.array[0] == 10);
            CHECK(mat.array[1] == 13);
            CHECK(mat.array[2] == 0);
            CHECK(mat.array[3] == 11);
            CHECK(mat.array[4] == 14);
            CHECK(mat.array[5] == 0);
            CHECK(mat.array[6] == 12);
            CHECK(mat.array[7] == 15);
            CHECK(mat.array[8] == 1);
        }
    }

    SECTION("Mat4")
    {
        SECTION("Type traits")
        {
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Glsl::Mat4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Glsl::Mat4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Glsl::Mat4));
            STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Glsl::Mat4));
        }

        static constexpr float data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        const sf::Glsl::Mat4   mat(data);
        CHECK(mat.array[0] == 1);
        CHECK(mat.array[1] == 2);
        CHECK(mat.array[2] == 3);
        CHECK(mat.array[3] == 4);
        CHECK(mat.array[4] == 5);
        CHECK(mat.array[5] == 6);
        CHECK(mat.array[6] == 7);
        CHECK(mat.array[7] == 8);
        CHECK(mat.array[8] == 9);
        CHECK(mat.array[9] == 10);
        CHECK(mat.array[10] == 11);
        CHECK(mat.array[11] == 12);
        CHECK(mat.array[12] == 13);
        CHECK(mat.array[13] == 14);
        CHECK(mat.array[14] == 15);
        CHECK(mat.array[15] == 16);
    }

    SECTION("Transform constructor")
    {
        constexpr sf::Transform transform(10, 11, 12, 13, 14, 15);
        const sf::Glsl::Mat4    mat = transform;
        CHECK(mat.array[0] == 10);
        CHECK(mat.array[1] == 13);
        CHECK(mat.array[2] == 0);
        CHECK(mat.array[3] == 0);
        CHECK(mat.array[4] == 11);
        CHECK(mat.array[5] == 14);
        CHECK(mat.array[6] == 0);
        CHECK(mat.array[7] == 0);
        CHECK(mat.array[8] == 0);
        CHECK(mat.array[9] == 0);
        CHECK(mat.array[10] == 1);
        CHECK(mat.array[11] == 0);
        CHECK(mat.array[12] == 12);
        CHECK(mat.array[13] == 15);
        CHECK(mat.array[14] == 0);
        CHECK(mat.array[15] == 1);
    }
}
