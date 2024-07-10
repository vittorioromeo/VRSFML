
#include <SFML/Window/GraphicsContext.hpp>

// Other 1st party headers
#include <SFML/Window/ContextSettings.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/UniquePtr.hpp>

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
};

}; // namespace sf::priv

struct TestContext
{
    TestContext(sf::GraphicsContext& theGraphicsContext) : glContext(theGraphicsContext.createGlContext())
    {
        if (!setActive(true))
            sf::priv::err() << "Failed to set context as active during construction" << sf::priv::errEndl;
    }

    ~TestContext()
    {
        if (glContext != nullptr && !setActive(false))
            sf::priv::err() << "Failed to set context as inactive during destruction" << sf::priv::errEndl;
    }

    [[nodiscard]] bool setActive(bool active) const
    {
        return sf::GraphicsContext::setActiveThreadLocalGlContext(*glContext, active);
    }

    TestContext(const TestContext&) = delete;

    TestContext& operator=(const TestContext&) = delete;

    TestContext(TestContext&& rhs) noexcept = default;

    TestContext& operator=(TestContext&& rhs) noexcept = default;

    [[nodiscard]] const sf::ContextSettings& getSettings() const
    {
        return glContext->getSettings();
    }

    sf::priv::UniquePtr<sf::priv::GlContext> glContext;
};

TEST_CASE("[Window] TestContext" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_copy_constructible_v<TestContext>);
        STATIC_CHECK(!std::is_copy_assignable_v<TestContext>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<TestContext>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<TestContext>);
    }

    SECTION("Construction")
    {
        const TestContext context(graphicsContext);
        CHECK(context.getSettings().majorVersion > 0);
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
        CHECK(sf::GraphicsContext::hasActiveThreadLocalGlContext());
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            SECTION("From active context")
            {
                TestContext       movedContext(graphicsContext);
                const TestContext context(SFML_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
                CHECK(sf::GraphicsContext::hasActiveThreadLocalGlContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());

                const TestContext context(SFML_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());
            }
        }

        SECTION("Assignment")
        {
            SECTION("From active context")
            {
                TestContext movedContext(graphicsContext);
                TestContext context(graphicsContext);
                CHECK(movedContext.setActive(true));
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == movedContext.glContext.get());
                CHECK(sf::GraphicsContext::hasActiveThreadLocalGlContext());

                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
                CHECK(sf::GraphicsContext::hasActiveThreadLocalGlContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());

                TestContext context(graphicsContext);
                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());
            }
        }
    }

    SECTION("setActive()")
    {
        TestContext context(graphicsContext);
        const auto  contextId = sf::GraphicsContext::getActiveThreadLocalGlContextId();

        // Set inactive
        CHECK(context.setActive(false));
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());

        // Set active
        CHECK(context.setActive(true));
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContextId() == contextId);

        // Create new context which becomes active automatically
        const TestContext newContext(graphicsContext);
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == newContext.glContext.get());
        const auto newContextId = sf::GraphicsContext::getActiveThreadLocalGlContextId();
        CHECK(newContextId != 0);

        // Set old context as inactive but new context remains active
        CHECK(context.setActive(false));
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == newContext.glContext.get());
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContextId() == newContextId);

        // Set old context as active again
        CHECK(context.setActive(true));
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContextId() == contextId);
    }

    SECTION("getActiveThreadLocalGlContext()/getActiveThreadLocalGlContextId()")
    {
        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());

        {
            const TestContext context(graphicsContext);
            CHECK(context.getSettings().majorVersion > 0);
            CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == context.glContext.get());
            CHECK(sf::GraphicsContext::hasActiveThreadLocalGlContext());
        }

        CHECK(sf::GraphicsContext::getActiveThreadLocalGlContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveThreadLocalGlContext());
    }

    SECTION("Version String")
    {
        TestContext context(graphicsContext);
        CHECK(context.setActive(true));

        using glGetStringFuncType  = const char*(GLAPI*)(unsigned int);
        const auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(
            sf::GraphicsContext::getFunction("glGetString"));
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
        CHECK(!graphicsContext.isExtensionAvailable("2024-04-01"));
        CHECK(!graphicsContext.isExtensionAvailable("let's assume this extension does not exist"));
    }

    SECTION("getFunction()")
    {
        const TestContext context(graphicsContext); // Windows requires an active context to use getFunction
        CHECK(sf::GraphicsContext::getFunction("glEnable"));
        CHECK(sf::GraphicsContext::getFunction("glGetError"));
        CHECK(sf::GraphicsContext::getFunction("glGetIntegerv"));
        CHECK(sf::GraphicsContext::getFunction("glGetString"));
        CHECK(sf::GraphicsContext::getFunction("glGetStringi"));
        CHECK(sf::GraphicsContext::getFunction("glIsEnabled"));
    }
}
