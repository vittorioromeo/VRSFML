#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsTriviallyCopyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyConstructible.hpp"
#include "Zancle/Trait/IsTriviallyMoveAssignable.hpp"
#include "Zancle/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::Vertex")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::Vertex));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::Vertex));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::Vertex));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::Vertex));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::Vertex));
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr za::Vertex vertex{};
            STATIC_CHECK(vertex.position == za::Vec2f(0.f, 0.f));
            STATIC_CHECK(vertex.color == za::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == za::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position")
        {
            constexpr za::Vertex vertex{{1.f, 2.f}};
            STATIC_CHECK(vertex.position == za::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == za::Color(255, 255, 255));
            STATIC_CHECK(vertex.texCoords == za::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position and color")
        {
            constexpr za::Vertex vertex{{1.f, 2.f}, {3, 4, 5, 6}};
            STATIC_CHECK(vertex.position == za::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == za::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == za::Vec2f(0.f, 0.f));
        }

        SECTION("Aggregate initialization -- Position, color, and coords")
        {
            constexpr za::Vertex vertex{{1.f, 2.f}, {3, 4, 5, 6}, {7.f, 8.f}};
            STATIC_CHECK(vertex.position == za::Vec2f(1.f, 2.f));
            STATIC_CHECK(vertex.color == za::Color(3, 4, 5, 6));
            STATIC_CHECK(vertex.texCoords == za::Vec2f(7.f, 8.f));
        }
    }
}
