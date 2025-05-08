#include "SFML/Config.hpp"

#ifdef SFML_SYSTEM_WINDOWS
// Other 1st party headers
    #include "SFML/Graphics/RenderWindow.hpp"

    #include "SFML/Window/Window.hpp"
    #include "SFML/Window/WindowBase.hpp"
    #include "SFML/Window/WindowContext.hpp"
    #include "SFML/Window/WindowHandle.hpp"

    #include "SFML/Base/Optional.hpp"

    #include <Doctest.hpp>

    #include <SystemUtil.hpp>

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>

namespace
{
bool gotWmShowWindow1 = false;

LRESULT WINAPI wndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_SHOWWINDOW:
            gotWmShowWindow1 = true;
            SFML_BASE_ASSERT(wParam == TRUE); // Window is being shown
            SFML_BASE_ASSERT(lParam == 0);    // Message comes from a ShowWindow call
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(handle, message, wParam, lParam);
}

sf::WindowHandle createWindowWrapper(LPWSTR className, HINSTANCE hInstance, DWORD dwExStyle, bool withMenu)
{
    HMENU hMenu = nullptr;
    if (withMenu)
    {
        hMenu = CreateMenu();
        AppendMenuW(hMenu, MF_STRING, 1, L"Test");
    }
    return CreateWindowExW(dwExStyle,
                           className,
                           L"WindowHandle Tests",
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           640,
                           480,
                           nullptr,
                           hMenu,
                           hInstance,
                           nullptr);
}

void runWindowTest(DWORD exStyle, bool withMenu)
{
    INFO("Running test with exStyle: " << exStyle << ", withMenu: " << withMenu);
    // Register a window class
    const WNDCLASSW classInfo{{},      // style
                              wndProc, // window procedure
                              {},      // class extra bytes
                              {},      // window extra bytes
                              GetModuleHandleW(nullptr),
                              {},
                              {},
                              {},
                              {},
                              L"sfml_WindowHandleTests"};
    const ATOM      winClassId = RegisterClassW(&classInfo);
    REQUIRE(winClassId);

    // Create the window using the provided parameters
    const sf::WindowHandle handle = createWindowWrapper(reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(winClassId)),
                                                        classInfo.hInstance,
                                                        exStyle,
                                                        withMenu);
    REQUIRE(handle);
    REQUIRE(!gotWmShowWindow1);
    REQUIRE(IsWindow(handle));

    RECT windowRect{};
    REQUIRE(GetClientRect(handle, &windowRect));
    ClientToScreen(handle, reinterpret_cast<LPPOINT>(&windowRect));
    const auto position = sf::Vec2(windowRect.left, windowRect.top).toVec2i();

    RECT clientRect{};
    REQUIRE(GetClientRect(handle, &clientRect));
    const auto initialSize = sf::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
    constexpr sf::Vec2u newSize(640, 480);

    // Test sf::WindowBase functionality
    SUBCASE("sf::WindowBase tests")
    {
        sf::base::Optional<sf::WindowBase> windowBase;

        windowBase.emplace(handle);

        INFO("sf::WindowBase test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(windowBase->getPosition() == position);
        CHECK(windowBase->getSize() == initialSize);
        CHECK(windowBase->getNativeHandle() == handle);

        CHECK(windowBase->getSize() != newSize);
        windowBase->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = sf::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);
        CHECK(windowBase->getSize() == size);
    }

    // Test sf::Window functionality
    SUBCASE("sf::Window tests")
    {
        sf::base::Optional<sf::Window> window;

        SUBCASE("Default context settings")
        {
            window.emplace(handle);

            INFO("sf::Window default context test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(window->getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SUBCASE("Custom context settings")
        {
            static constexpr sf::ContextSettings contextSettings{1, 1, 1};

            window.emplace(handle, contextSettings);

            INFO("sf::Window custom context test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(window->getSettings().depthBits >= 1);
            CHECK(window->getSettings().stencilBits >= 1);
            CHECK(window->getSettings().antiAliasingLevel >= 1);
        }

        INFO("sf::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(window->getPosition() == position);
        CHECK(window->getSize() == initialSize);
        CHECK(window->getNativeHandle() == handle);

        CHECK(window->getSize() != newSize);
        window->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = sf::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);
        CHECK(window->getSize() == size);
    }

    SECTION("sf::RenderWindow")
    {
        sf::base::Optional<sf::RenderWindow> renderWindow;

        SECTION("Default context settings")
        {
            renderWindow.emplace(handle);

            INFO("sf::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(renderWindow->getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Custom context settings")
        {
            static constexpr sf::ContextSettings contextSettings{/* depthBits*/ 1,
                                                                 /* stencilBits */ 1,
                                                                 /* antiAliasingLevel */ 1};

            renderWindow.emplace(handle, contextSettings);

            INFO("sf::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(renderWindow->getSettings().depthBits >= 1);
            CHECK(renderWindow->getSettings().stencilBits >= 1);
            CHECK(renderWindow->getSettings().antiAliasingLevel >= 1);
        }

        INFO("sf::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(renderWindow->getPosition() == position);
        CHECK(renderWindow->getSize() == initialSize);
        CHECK(renderWindow->getNativeHandle() == handle);

        CHECK(renderWindow->getSize() != newSize);
        renderWindow->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = sf::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);                 // Validate that the actual client rect is indeed what we asked for
        CHECK(renderWindow->getSize() == size); // Validate that the `getSize` also returns the _actual_ client size
    }

    INFO("Final checks with exStyle: " << exStyle << ", withMenu: " << withMenu);
    CHECK(gotWmShowWindow1);
    CHECK(IsWindow(handle));

    CHECK(DestroyWindow(handle));
    CHECK(UnregisterClassW(classInfo.lpszClassName, classInfo.hInstance));
    gotWmShowWindow1 = false;
    CHECK(!IsWindow(handle));
}
} // anonymous namespace

TEST_CASE("[Window] sf::WindowHandle (Win32) - Parameterized")
{
    auto windowContext = sf::WindowContext::create().value();

    SUBCASE("exStyle = 0, withMenu = false")
    {
        runWindowTest(0, false);
    }

    SUBCASE("exStyle = WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE, withMenu = false")
    {
        runWindowTest(WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE, false);
    }

    SUBCASE("exStyle = 0, withMenu = true")
    {
        runWindowTest(0, true);
    }

    SUBCASE("exStyle = WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE, withMenu = true")
    {
        runWindowTest(WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE, true);
    }
}

#endif
