
#include "Zancle/Window/WindowContext.hpp"

// Other 1st party headers
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/ContextSettings.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"

#include "Zancle/Base/Macros.hpp"

#if defined(ZA_SYSTEM_WINDOWS)
    #define GLAPI __stdcall
#else
    #define GLAPI
#endif

////////////////////////////////////////////////////////////
#define protected public
#include "../src/Zancle/GLUtils/GlContext.hpp"
#undef protected


struct TestContext
{
    decltype(za::WindowContext::createGlContext(za::ContextSettings{})) glContext;

    TestContext() : glContext(za::WindowContext::createGlContext(za::ContextSettings{}))
    {
        if (!setActive(true))
            za::priv::errMsg("Failed to set context as active during construction");
    }

    ~TestContext()
    {
        if (glContext != nullptr && !setActive(false))
            za::priv::errMsg("Failed to set context as inactive during destruction");
    }

    [[nodiscard]] bool setActive(bool active) const
    {
        return za::WindowContext::setActiveThreadLocalGlContext(*glContext, active);
    }

    TestContext(const TestContext&) = delete;

    TestContext& operator=(const TestContext&) = delete;

    TestContext(TestContext&& rhs) noexcept = default;

    TestContext& operator=(TestContext&& rhs) noexcept = default;

    [[nodiscard]] const za::ContextSettings& getSettings() const
    {
        return glContext->getSettings();
    }

    [[nodiscard]] static unsigned int getActiveThreadLocalGlContextId()
    {
        return za::WindowContext::getActiveThreadLocalGlContextId();
    }

    [[nodiscard]] static bool hasActiveThreadLocalGlContext()
    {
        return za::WindowContext::hasActiveThreadLocalGlContext();
    }

    [[nodiscard]] static bool isActiveGlContextSharedContext()
    {
        return za::WindowContext::isActiveGlContextSharedContext();
    }

    [[nodiscard]] static bool isExtensionAvailable(const char* name)
    {
        return za::WindowContext::isExtensionAvailable(name);
    }

    [[nodiscard]] static auto getFunction(const char* name)
    {
        return za::WindowContext::getFunction(name);
    }
};

TEST_CASE("[Window] TestContext" * tst::skip(skipDisplayTests))
{
    {
        CHECK(!za::WindowContext::isInstalled());
        auto windowContext = za::WindowContext::create().value();
        CHECK(za::WindowContext::isInstalled());
        CHECK(TestContext::hasActiveThreadLocalGlContext());
        CHECK(TestContext::isActiveGlContextSharedContext());

        SECTION("Type traits")
        {
            STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(TestContext));
            STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(TestContext));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(TestContext));
            STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(TestContext));
        }

        SECTION("Construction")
        {
            {
                const TestContext context;
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(TestContext::hasActiveThreadLocalGlContext());
            }

            CHECK(TestContext::hasActiveThreadLocalGlContext());
            CHECK(TestContext::isActiveGlContextSharedContext());
        }

        SECTION("Move semantics")
        {
            SECTION("Construction")
            {
                SECTION("From active context")
                {
                    TestContext       movedContext;
                    const TestContext context(ZA_MOVE(movedContext));
                    CHECK(context.getSettings().majorVersion > 0);
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                    CHECK(TestContext::hasActiveThreadLocalGlContext());
                }

                SECTION("From inactive context")
                {
                    TestContext movedContext;
                    CHECK(movedContext.setActive(false));
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
                    CHECK(TestContext::isActiveGlContextSharedContext());

                    const TestContext context(ZA_MOVE(movedContext));
                    CHECK(context.getSettings().majorVersion > 0);
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
                    CHECK(TestContext::isActiveGlContextSharedContext());
                }
            }

            SECTION("Assignment")
            {
                SECTION("From active context")
                {
                    TestContext movedContext;
                    TestContext context;
                    CHECK(movedContext.setActive(true));
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == movedContext.glContext->getId());
                    CHECK(TestContext::hasActiveThreadLocalGlContext());

                    context = ZA_MOVE(movedContext);
                    CHECK(context.getSettings().majorVersion > 0);
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                    CHECK(TestContext::hasActiveThreadLocalGlContext());
                }

                SECTION("From inactive context")
                {
                    TestContext movedContext;
                    CHECK(movedContext.setActive(false));
                    CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);

                    TestContext context;
                    CHECK(context.setActive(false));
                    context = ZA_MOVE(movedContext);
                    CHECK(context.getSettings().majorVersion > 0);
                    CHECK(TestContext::isActiveGlContextSharedContext());
                }
            }
        }

        SECTION("setActive()")
        {
            {
                TestContext context;
                const auto  contextId = TestContext::getActiveThreadLocalGlContextId();

                // Set inactive
                CHECK(context.setActive(false));
                CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
                CHECK(TestContext::isActiveGlContextSharedContext());

                // Set active
                CHECK(context.setActive(true));
                CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(TestContext::getActiveThreadLocalGlContextId() == contextId);

                // Create new context which becomes active automatically
                const TestContext newContext;
                CHECK(TestContext::getActiveThreadLocalGlContextId() == newContext.glContext->getId());
                const auto newContextId = TestContext::getActiveThreadLocalGlContextId();
                CHECK(newContextId != 0);

                // Set old context as inactive but new context remains active
                CHECK(context.setActive(false));
                CHECK(TestContext::getActiveThreadLocalGlContextId() == newContext.glContext->getId());
                CHECK(TestContext::getActiveThreadLocalGlContextId() == newContextId);

                // Set old context as active again
                CHECK(context.setActive(true));
                CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(TestContext::getActiveThreadLocalGlContextId() == contextId);
            }

            CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
            CHECK(TestContext::isActiveGlContextSharedContext());
        }

        SECTION("getActiveThreadLocalGlContextId()/getActiveThreadLocalGlContextId()")
        {
            {
                const TestContext context;
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(TestContext::getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(TestContext::hasActiveThreadLocalGlContext());
            }

            CHECK(TestContext::getActiveThreadLocalGlContextId() == 1u);
            CHECK(TestContext::isActiveGlContextSharedContext());
        }

        SECTION("Version String")
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

        SECTION("isExtensionAvailable()")
        {
            CHECK(!TestContext::isExtensionAvailable("2024-04-01"));
            CHECK(!TestContext::isExtensionAvailable("let's assume this extension does not exist"));
        }

        SECTION("getFunction()")
        {
            const TestContext context; // Windows requires an active context to use getFunction
            CHECK(TestContext::getFunction("glEnable"));
            CHECK(TestContext::getFunction("glGetError"));
            CHECK(TestContext::getFunction("glGetIntegerv"));
            CHECK(TestContext::getFunction("glGetString"));
            CHECK(TestContext::getFunction("glGetStringi"));
            CHECK(TestContext::getFunction("glIsEnabled"));
        }
    }

    CHECK(!za::WindowContext::isInstalled());
}
