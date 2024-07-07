#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/Window/Context.hpp>

// Other 1st party headers
#include <SFML/Window/ContextSettings.hpp>

#include <SFML/System/Macros.hpp>

#include <Doctest.hpp>

#include <WindowUtil.hpp>

#include <string>
#include <type_traits>

#if defined(SFML_SYSTEM_WINDOWS)
#define GLAPI __stdcall
#else
#define GLAPI
#endif

TEST_CASE("[Window] sf::Context" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Context>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Context>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Context>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Context>);
    }

    SECTION("Construction")
    {
        const sf::Context context(graphicsContext);
        CHECK(context.getSettings().majorVersion > 0);
        CHECK(sf::Context::getActiveContext() == &context);
        CHECK(sf::Context::hasActiveContext());
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            SECTION("From active context")
            {
                sf::Context       movedContext(graphicsContext);
                const sf::Context context(SFML_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::Context::getActiveContext() == &context);
                CHECK(sf::Context::hasActiveContext());
            }

            SECTION("From inactive context")
            {
                sf::Context movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(sf::Context::getActiveContext() == nullptr);
                CHECK(!sf::Context::hasActiveContext());

                const sf::Context context(SFML_MOVE(movedContext));
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::Context::getActiveContext() == nullptr);
                CHECK(!sf::Context::hasActiveContext());
            }
        }

        SECTION("Assignment")
        {
            SECTION("From active context")
            {
                sf::Context movedContext(graphicsContext);
                sf::Context context(graphicsContext);
                CHECK(movedContext.setActive(true));
                CHECK(sf::Context::getActiveContext() == &movedContext);
                CHECK(sf::Context::hasActiveContext());

                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::Context::getActiveContext() == &context);
                CHECK(sf::Context::hasActiveContext());
            }

            SECTION("From inactive context")
            {
                sf::Context movedContext(graphicsContext);
                CHECK(movedContext.setActive(false));
                CHECK(sf::Context::getActiveContext() == nullptr);
                CHECK(!sf::Context::hasActiveContext());

                sf::Context context(graphicsContext);
                context = SFML_MOVE(movedContext);
                CHECK(context.getSettings().majorVersion > 0);
                CHECK(sf::Context::getActiveContext() == nullptr);
                CHECK(!sf::Context::hasActiveContext());
            }
        }
    }

    SECTION("setActive()")
    {
        sf::Context context(graphicsContext);
        const auto  contextId = sf::Context::getActiveContextId();

        // Set inactive
        CHECK(context.setActive(false));
        CHECK(sf::Context::getActiveContext() == nullptr);
        CHECK(!sf::Context::hasActiveContext());

        // Set active
        CHECK(context.setActive(true));
        CHECK(sf::Context::getActiveContext() == &context);
        CHECK(sf::Context::getActiveContextId() == contextId);

        // Create new context which becomes active automatically
        const sf::Context newContext(graphicsContext);
        CHECK(sf::Context::getActiveContext() == &newContext);
        const auto newContextId = sf::Context::getActiveContextId();
        CHECK(newContextId != 0);

        // Set old context as inactive but new context remains active
        CHECK(context.setActive(false));
        CHECK(sf::Context::getActiveContext() == &newContext);
        CHECK(sf::Context::getActiveContextId() == newContextId);

        // Set old context as active again
        CHECK(context.setActive(true));
        CHECK(sf::Context::getActiveContext() == &context);
        CHECK(sf::Context::getActiveContextId() == contextId);
    }

    SECTION("getActiveContext()/getActiveContextId()")
    {
        CHECK(sf::Context::getActiveContext() == nullptr);
        CHECK(!sf::Context::hasActiveContext());

        {
            const sf::Context context(graphicsContext);
            CHECK(context.getSettings().majorVersion > 0);
            CHECK(sf::Context::getActiveContext() == &context);
            CHECK(sf::Context::hasActiveContext());
        }

        CHECK(sf::Context::getActiveContext() == nullptr);
        CHECK(!sf::Context::hasActiveContext());
    }

    SECTION("Version String")
    {
        sf::Context context(graphicsContext);
        CHECK(context.setActive(true));

        using glGetStringFuncType  = const char*(GLAPI*)(unsigned int);
        const auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(sf::Context::getFunction("glGetString"));
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
        CHECK(!sf::Context::isExtensionAvailable("2024-04-01"));
        CHECK(!sf::Context::isExtensionAvailable("let's assume this extension does not exist"));
    }

    SECTION("getFunction()")
    {
        const sf::Context context(graphicsContext); // Windows requires an active context to use getFunction
        CHECK(sf::Context::getFunction("glEnable"));
        CHECK(sf::Context::getFunction("glGetError"));
        CHECK(sf::Context::getFunction("glGetIntegerv"));
        CHECK(sf::Context::getFunction("glGetString"));
        CHECK(sf::Context::getFunction("glGetStringi"));
        CHECK(sf::Context::getFunction("glIsEnabled"));
    }
}
