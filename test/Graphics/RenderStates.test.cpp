#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/RenderStates.hpp"

#include "Zancle/Graphics/BlendMode.hpp"
#include "Zancle/Graphics/View.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"

TEST_CASE("[Graphics] za::RenderStates")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::RenderStates));

        STATIC_CHECK(!ZB_IS_TRIVIAL(za::RenderStates)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::RenderStates));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::RenderStates));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::RenderStates, za::RenderStates));

        STATIC_CHECK(sizeof(za::RenderStates) <= 128); // TODO P0: this is big...
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const za::RenderStates renderStates{};
            CHECK(renderStates.blendMode == za::BlendAlpha);
            CHECK(renderStates.stencilMode == za::StencilMode{});
            CHECK(renderStates.transform == za::Transform{});
            CHECK(renderStates.view == za::View{});
            CHECK(renderStates.texture == nullptr);
            CHECK(renderStates.shader == nullptr);
        }
    }

    SECTION("Default constant")
    {
        CHECK(za::RenderStates{}.blendMode == za::BlendAlpha);
        CHECK(za::RenderStates{}.stencilMode == za::StencilMode{});
        CHECK(za::RenderStates{}.transform == za::Transform{});
        CHECK(za::RenderStates{}.view == za::View{});
        CHECK(za::RenderStates{}.texture == nullptr);
        CHECK(za::RenderStates{}.shader == nullptr);
    }
}
