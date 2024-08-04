
#include <SFML/Window/WindowContext.hpp>

// Other 1st party headers
#include <SFML/Window/ContextSettings.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <Doctest.hpp>

#include <WindowUtil.hpp>

#include <string>
#include <type_traits>

#if defined(SFML_SYSTEM_WINDOWS)
#define GLAPI __stdcall
#else
#define GLAPI
#endif

namespace sf::priv
{
class GlContext
{
public:
    [[nodiscard]] const ContextSettings& getSettings() const;
    [[nodiscard]] std::uint64_t          getId() const;
};

}; // namespace sf::priv

struct TestContext
{
    TestContext(sf::WindowContext& theWindowContext) :
    windowContext(&theWindowContext),
    glContext(theWindowContext.createGlContext())
    {
        if (!setActive(true))
            sf::priv::err() << "Failed to set context as active during construction";
    }

    ~TestContext()
    {
        if (glContext != nullptr && !setActive(false))
            sf::priv::err() << "Failed to set context as inactive during destruction";
    }

    [[nodiscard]] bool setActive(bool active) const
    {
        return windowContext->setActiveThreadLocalGlContext(*glContext, active);
    }

    TestContext(const TestContext&) = delete;

    TestContext& operator=(const TestContext&) = delete;

    TestContext(TestContext&& rhs) noexcept = default;

    TestContext& operator=(TestContext&& rhs) noexcept = default;

    [[nodiscard]] const sf::ContextSettings& getSettings() const
    {
        return glContext->getSettings();
    }

    sf::WindowContext*                       windowContext;
    sf::base::UniquePtr<sf::priv::GlContext> glContext;
};

TEST_CASE("[Window] TestContext" * doctest::skip(skipDisplayTests))
{
    sf::WindowContext windowContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_copy_constructible_v<TestContext>);
        STATIC_CHECK(!std::is_copy_assignable_v<TestContext>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<TestContext>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<TestContext>);
    }

    SECTION("Construction")
    {
        const TestContext context(windowContext);
        CHECK(context.getSettings().majorVersion > 0);
        CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
        CHECK(windowContext.hasActiveThreadLocalGlContext());
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            SECTION("From active context")
            {
                TestContext       movedContext(windowContext);
                const TestContext context(SFML_BASE_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(windowContext.hasActiveThreadLocalGlContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(windowContext);
                CHECK(movedContext.setActive(false));
                CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
                CHECK(!windowContext.hasActiveThreadLocalGlContext());

                const TestContext context(SFML_BASE_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
                CHECK(!windowContext.hasActiveThreadLocalGlContext());
            }
        }

        SECTION("Assignment")
        {
            SECTION("From active context")
            {
                TestContext movedContext(windowContext);
                TestContext context(windowContext);
                CHECK(movedContext.setActive(true));
                CHECK(windowContext.getActiveThreadLocalGlContextId() == movedContext.glContext->getId());
                CHECK(windowContext.hasActiveThreadLocalGlContext());

                context = SFML_BASE_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
                CHECK(windowContext.hasActiveThreadLocalGlContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(windowContext);
                CHECK(movedContext.setActive(false));
                CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
                CHECK(!windowContext.hasActiveThreadLocalGlContext());

                TestContext context(windowContext);
                context = SFML_BASE_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
                CHECK(!windowContext.hasActiveThreadLocalGlContext());
            }
        }
    }

    SECTION("setActive()")
    {
        TestContext context(windowContext);
        const auto  contextId = windowContext.getActiveThreadLocalGlContextId();

        // Set inactive
        CHECK(context.setActive(false));
        CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
        CHECK(!windowContext.hasActiveThreadLocalGlContext());

        // Set active
        CHECK(context.setActive(true));
        CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
        CHECK(windowContext.getActiveThreadLocalGlContextId() == contextId);

        // Create new context which becomes active automatically
        const TestContext newContext(windowContext);
        CHECK(windowContext.getActiveThreadLocalGlContextId() == newContext.glContext->getId());
        const auto newContextId = windowContext.getActiveThreadLocalGlContextId();
        CHECK(newContextId != 0);

        // Set old context as inactive but new context remains active
        CHECK(context.setActive(false));
        CHECK(windowContext.getActiveThreadLocalGlContextId() == newContext.glContext->getId());
        CHECK(windowContext.getActiveThreadLocalGlContextId() == newContextId);

        // Set old context as active again
        CHECK(context.setActive(true));
        CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
        CHECK(windowContext.getActiveThreadLocalGlContextId() == contextId);
    }

    SECTION("getActiveThreadLocalGlContextId()/getActiveThreadLocalGlContextId()")
    {
        CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
        CHECK(!windowContext.hasActiveThreadLocalGlContext());

        {
            const TestContext context(windowContext);
            CHECK(context.getSettings().majorVersion > 0);
            CHECK(windowContext.getActiveThreadLocalGlContextId() == context.glContext->getId());
            CHECK(windowContext.hasActiveThreadLocalGlContext());
        }

        CHECK(windowContext.getActiveThreadLocalGlContextId() == 0u);
        CHECK(!windowContext.hasActiveThreadLocalGlContext());
    }

    SECTION("Version String")
    {
        TestContext context(windowContext);
        CHECK(context.setActive(true));

        using glGetStringFuncType  = const char*(GLAPI*)(unsigned int);
        const auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(windowContext.getFunction("glGetString"));
        REQUIRE(glGetStringFunc);

        constexpr unsigned int glVendor   = 0x1F00;
        constexpr unsigned int glRenderer = 0x1F01;
        constexpr unsigned int glVersion  = 0x1F02;

        const char* vendor   = glGetStringFunc(glVendor);
        const char* renderer = glGetStringFunc(glRenderer);
        const char* version  = glGetStringFunc(glVersion);

        REQUIRE(vendor != nullptr);
        REQUIRE(renderer != nullptr);
        REQUIRE(version != nullptr);

        SUCCEED(std::string("OpenGL vendor: ") + vendor);
        SUCCEED(std::string("OpenGL renderer: ") + renderer);
        SUCCEED(std::string("OpenGL version: ") + version);
    }

    SECTION("isExtensionAvailable()")
    {
        CHECK(!windowContext.isExtensionAvailable("2024-04-01"));
        CHECK(!windowContext.isExtensionAvailable("let's assume this extension does not exist"));
    }

    SECTION("getFunction()")
    {
        const TestContext context(windowContext); // Windows requires an active context to use getFunction
        CHECK(windowContext.getFunction("glEnable"));
        CHECK(windowContext.getFunction("glGetError"));
        CHECK(windowContext.getFunction("glGetIntegerv"));
        CHECK(windowContext.getFunction("glGetString"));
        CHECK(windowContext.getFunction("glGetStringi"));
        CHECK(windowContext.getFunction("glIsEnabled"));
    }
}
