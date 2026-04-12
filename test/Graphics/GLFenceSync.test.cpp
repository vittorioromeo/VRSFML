#include "WindowUtil.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"

#include "SFML/GLUtils/FenceUtils.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLFenceSync.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Window/WindowContext.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"

#include <Doctest.hpp>


#ifndef SFML_OPENGL_ES

namespace
{
using Fence = sf::priv::GLFenceSync;


////////////////////////////////////////////////////////////
void drainGLCommandQueue()
{
    glCheck(glFinish());
}

} // namespace


TEST_CASE("[GLUtils] sf::priv::GLFenceSync / FenceUtils" * doctest::skip(skipDisplayTests))
{
    CHECK(!sf::WindowContext::isInstalled());
    CHECK(!sf::GraphicsContext::isInstalled());

    auto graphicsContext = sf::GraphicsContext::create().value();

    CHECK(sf::WindowContext::isInstalled());
    CHECK(sf::GraphicsContext::isInstalled());

    SECTION("Type traits")
    {
        STATIC_CHECK(!sf::base::isCopyConstructible<Fence>);
        STATIC_CHECK(!sf::base::isCopyAssignable<Fence>);
        STATIC_CHECK(sf::base::isNoThrowMoveConstructible<Fence>);
        STATIC_CHECK(sf::base::isNoThrowMoveAssignable<Fence>);
    }

    SECTION("Default-constructed state has no native fence")
    {
        Fence fence;
        CHECK(!fence.hasValue());
        CHECK(!static_cast<bool>(fence));
        CHECK(fence.getNativeHandle() == nullptr);
    }

    SECTION("reset and deleteFenceIfNeeded are safe no-ops on an empty fence")
    {
        Fence fence;

        fence.reset();
        CHECK(!fence);

        sf::priv::deleteFenceIfNeeded(fence);
        CHECK(!fence);
    }

    SECTION("makeFence returns an owning fence")
    {
        Fence fence = sf::priv::makeFence();
        CHECK(fence.hasValue());
        CHECK(static_cast<bool>(fence));
        CHECK(fence.getNativeHandle() != nullptr);
    }

    SECTION("deleteFenceIfNeeded consumes the fence and is idempotent")
    {
        Fence fence = sf::priv::makeFence();
        REQUIRE(static_cast<bool>(fence));

        sf::priv::deleteFenceIfNeeded(fence);
        CHECK(!fence);

        sf::priv::deleteFenceIfNeeded(fence);
        CHECK(!fence);
    }

    SECTION("waitOnFence on an empty fence is a no-op")
    {
        Fence fence;
        sf::priv::waitOnFence(fence);
        CHECK(!fence);
    }

    SECTION("waitOnFence consumes a signaled fence")
    {
        Fence fence = sf::priv::makeFence();
        REQUIRE(static_cast<bool>(fence));

        drainGLCommandQueue();
        sf::priv::waitOnFence(fence);

        CHECK(!fence);
    }

    SECTION("tryWaitOnFence returns true for an empty fence")
    {
        Fence fence;
        CHECK(sf::priv::tryWaitOnFence(fence));
        CHECK(!fence);
    }

    SECTION("tryWaitOnFence consumes a signaled fence")
    {
        Fence fence = sf::priv::makeFence();
        REQUIRE(static_cast<bool>(fence));

        drainGLCommandQueue();
        CHECK(sf::priv::tryWaitOnFence(fence));
        CHECK(!fence);
    }

    SECTION("Move construction transfers ownership")
    {
        Fence source = sf::priv::makeFence();
        REQUIRE(static_cast<bool>(source));

        const void* const originalHandle = source.getNativeHandle();
        Fence             dest{SFML_BASE_MOVE(source)};

        CHECK(!source);
        CHECK(static_cast<bool>(dest));
        CHECK(dest.getNativeHandle() == originalHandle);

        sf::priv::deleteFenceIfNeeded(dest);
    }

    SECTION("Move assignment transfers ownership and clears the source")
    {
        Fence source = sf::priv::makeFence();
        Fence dest   = sf::priv::makeFence();

        REQUIRE(static_cast<bool>(source));
        REQUIRE(static_cast<bool>(dest));

        const void* const sourceHandle = source.getNativeHandle();
        dest                           = SFML_BASE_MOVE(source);

        CHECK(!source);
        CHECK(static_cast<bool>(dest));
        CHECK(dest.getNativeHandle() == sourceHandle);

        sf::priv::deleteFenceIfNeeded(dest);
    }
}

#endif // !SFML_OPENGL_ES
