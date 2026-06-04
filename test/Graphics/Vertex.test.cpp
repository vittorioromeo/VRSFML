#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/Graphics/Vertex.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::Vertex")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::Vertex));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::Vertex));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::Vertex));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::Vertex));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::Vertex));
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
