
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
    [[nodiscard]] bool                   setActive(bool active);
    [[nodiscard]] const ContextSettings& getSettings() const;
};

}; // namespace sf::priv

struct TestContext
{
    TestContext(sf::GraphicsContext& graphicsContext) : context(graphicsContext.createGlContext())
    {
        if (!context->setActive(true))
            sf::priv::err() << "Failed to set context as active during construction" << sf::priv::errEndl;
    }

    ~TestContext()
    {
        if (context && !context->setActive(false))
            sf::priv::err() << "Failed to set context as inactive during destruction" << sf::priv::errEndl;
    }

    [[nodiscard]] bool setActive(bool active) const
    {
        return context->setActive(active);
    }

    TestContext(const TestContext&) = delete;

    TestContext& operator=(const TestContext&) = delete;

    TestContext(TestContext&& rhs) noexcept = default;

    TestContext& operator=(TestContext&& rhs) noexcept = default;

    [[nodiscard]] const sf::ContextSettings& getSettings() const
    {
        return context->getSettings();
    }

    sf::priv::UniquePtr<sf::priv::GlContext> context;
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
        CHECK(graphicsContext.getActiveContext() == context.context.get());
        CHECK(sf::GraphicsContext::hasActiveContext());
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
                CHECK(graphicsContext.getActiveContext() == context.context.get());
                CHECK(sf::GraphicsContext::hasActiveContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(graphicsContext.getActiveContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveContext());

                const TestContext context(SFML_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(graphicsContext.getActiveContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveContext());
            }
        }

        SECTION("Assignment")
        {
            SECTION("From active context")
            {
                TestContext movedContext(graphicsContext);
                TestContext context(graphicsContext);
                CHECK(movedContext.setActive(true));
                CHECK(graphicsContext.getActiveContext() == movedContext.context.get());
                CHECK(sf::GraphicsContext::hasActiveContext());

                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(graphicsContext.getActiveContext() == context.context.get());
                CHECK(sf::GraphicsContext::hasActiveContext());
            }

            SECTION("From inactive context")
            {
                TestContext movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(graphicsContext.getActiveContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveContext());

                TestContext context(graphicsContext);
                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(graphicsContext.getActiveContext() == nullptr);
                CHECK(!sf::GraphicsContext::hasActiveContext());
            }
        }
    }

    SECTION("setActive()")
    {
        TestContext context(graphicsContext);
        const auto  contextId = sf::GraphicsContext::getActiveContextId();

        // Set inactive
        CHECK(context.setActive(false));
        CHECK(graphicsContext.getActiveContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveContext());

        // Set active
        CHECK(context.setActive(true));
        CHECK(graphicsContext.getActiveContext() == context.context.get());
        CHECK(sf::GraphicsContext::getActiveContextId() == contextId);

        // Create new context which becomes active automatically
        const TestContext newContext(graphicsContext);
        CHECK(graphicsContext.getActiveContext() == newContext.context.get());
        const auto newContextId = sf::GraphicsContext::getActiveContextId();
        CHECK(newContextId != 0);

        // Set old context as inactive but new context remains active
        CHECK(context.setActive(false));
        CHECK(graphicsContext.getActiveContext() == newContext.context.get());
        CHECK(sf::GraphicsContext::getActiveContextId() == newContextId);

        // Set old context as active again
        CHECK(context.setActive(true));
        CHECK(graphicsContext.getActiveContext() == context.context.get());
        CHECK(sf::GraphicsContext::getActiveContextId() == contextId);
    }

    SECTION("getActiveContext()/getActiveContextId()")
    {
        CHECK(graphicsContext.getActiveContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveContext());

        {
            const TestContext context(graphicsContext);
            CHECK(context.getSettings().majorVersion > 0);
            CHECK(graphicsContext.getActiveContext() == context.context.get());
            CHECK(sf::GraphicsContext::hasActiveContext());
        }

        CHECK(graphicsContext.getActiveContext() == nullptr);
        CHECK(!sf::GraphicsContext::hasActiveContext());
    }

    SECTION("Version String")
    {
        TestContext context(graphicsContext);
        CHECK(context.setActive(true));

        using glGetStringFuncType  = const char*(GLAPI*)(unsigned int);
        const auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(graphicsContext.getFunction("glGetString"));
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
        CHECK(graphicsContext.getFunction("glEnable"));
        CHECK(graphicsContext.getFunction("glGetError"));
        CHECK(graphicsContext.getFunction("glGetIntegerv"));
        CHECK(graphicsContext.getFunction("glGetString"));
        CHECK(graphicsContext.getFunction("glGetStringi"));
        CHECK(graphicsContext.getFunction("glIsEnabled"));
    }
}
