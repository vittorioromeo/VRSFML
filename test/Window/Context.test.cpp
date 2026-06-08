
#include "Zancle/Window/WindowContext.hpp"

// Other 1st party headers
#include "TestContext.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/ContextSettings.hpp"

#if defined(ZA_SYSTEM_WINDOWS)
    #define GLAPI __stdcall
#else
    #define GLAPI
#endif


////////////////////////////////////////////////////////////
TEST_CASE("[Window] WindowContext install/uninstall lifecycle" * tst::skip(skipDisplayTests))
{
    // The only place in the suite that asserts `WindowContext::isInstalled()`
    // tracks the create/destroy cycle. Deliberately does NOT use
    // `TST_CASE_SHARED` -- the whole point is to exercise the local-scope
    // RAII path.
    CHECK(!za::WindowContext::isInstalled());
    {
        auto windowContext = za::WindowContext::create().value();
        CHECK(za::WindowContext::isInstalled());
        CHECK(TestContext::hasActiveThreadLocalGlContext());
        CHECK(TestContext::isActiveGlContextSharedContext());
    }
    CHECK(!za::WindowContext::isInstalled());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Window] WindowContext GL context API" * tst::skip(skipDisplayTests))
{
    // Shared install across all sections -- the lifecycle assertions
    // already live in the dedicated test case above, so we don't need
    // to re-prove install/uninstall here.
    [[maybe_unused]] auto& windowContext = TST_CASE_SHARED(za::WindowContext::create().value());

    SECTION("Fresh context activates on construction")
    {
        const TestContext context;
        CHECK(context.getSettings().majorVersion > 0);
        CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
        CHECK(TestContext::hasActiveThreadLocalGlContext());
    }

    SECTION("Destroying a fresh context restores the shared one as active")
    {
        {
            const TestContext context;
            CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
        }
        // After destruction the shared context (id 1) becomes active again.
        CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
        CHECK(TestContext::isActiveGlContextSharedContext());
    }

    SECTION("setActive() swaps the active thread-local context")
    {
        TestContext context;
        const auto  contextId = TestContext::getActiveThreadLocalGlContextId();

        // Set inactive -- the shared context (id 1) takes over.
        CHECK(context.setActive(false));
        CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
        CHECK(TestContext::isActiveGlContextSharedContext());

        // Set active again.
        CHECK(context.setActive(true));
        CHECK(TestContext::getActiveThreadLocalGlContextId() == contextId);

        // Creating a second context auto-activates it.
        const TestContext newContext;
        CHECK(TestContext::getActiveThreadLocalGlContextId() == newContext.glContext->getId());
        const auto newContextId = TestContext::getActiveThreadLocalGlContextId();
        CHECK(newContextId != 0);
        CHECK(newContextId != contextId);

        // Deactivating `context` while `newContext` is still active is a no-op
        // for the active id.
        CHECK(context.setActive(false));
        CHECK(TestContext::getActiveThreadLocalGlContextId() == newContextId);

        // Re-activating `context` swaps it back in.
        CHECK(context.setActive(true));
        CHECK(TestContext::getActiveThreadLocalGlContextId() == contextId);
    }

    SECTION("Active context id reverts to the shared context (id 1) after scope exit")
    {
        {
            const TestContext context;
            CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
            CHECK(TestContext::hasActiveThreadLocalGlContext());
        }
        CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
        CHECK(TestContext::isActiveGlContextSharedContext());
    }

    SECTION("getFunction() returns a working glGetString")
    {
        TestContext context;
        CHECK(context.setActive(true));

        using glGetStringFuncType  = const char*(GLAPI*)(unsigned int);
        const auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(TestContext::getFunction("glGetString"));
        REQUIRE(glGetStringFunc);

        constexpr unsigned int glVendor   = 0x1F'00;
        constexpr unsigned int glRenderer = 0x1F'01;
        constexpr unsigned int glVersion  = 0x1F'02;

        const char* vendor   = glGetStringFunc(glVendor);
        const char* renderer = glGetStringFunc(glRenderer);
        const char* version  = glGetStringFunc(glVersion);

        REQUIRE(vendor != nullptr);
        REQUIRE(renderer != nullptr);
        REQUIRE(version != nullptr);

        MESSAGE("OpenGL vendor: ", vendor);
        MESSAGE("OpenGL renderer: ", renderer);
        MESSAGE("OpenGL version: ", version);
    }

    SECTION("isExtensionAvailable() returns false for unknown extensions")
    {
        CHECK(!TestContext::isExtensionAvailable("2024-04-01"));
        CHECK(!TestContext::isExtensionAvailable("let's assume this extension does not exist"));
    }

    SECTION("getFunction() returns non-null for standard GL entry points")
    {
        const TestContext context; // Windows requires an active context to use getFunction.
        CHECK(TestContext::getFunction("glEnable"));
        CHECK(TestContext::getFunction("glGetError"));
        CHECK(TestContext::getFunction("glGetIntegerv"));
        CHECK(TestContext::getFunction("glGetString"));
        CHECK(TestContext::getFunction("glGetStringi"));
        CHECK(TestContext::getFunction("glIsEnabled"));
    }
}
