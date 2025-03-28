#include "SFML/Config.hpp"

#ifdef SFML_SYSTEM_WINDOWS

    #include "SFML/Window/WindowHandle.hpp"

    // Other 1st party headers
    #include "SFML/Window/Window.hpp"
    #include "SFML/Window/WindowBase.hpp"
    #include "SFML/Window/WindowContext.hpp"

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
    const auto position = sf::Vector2(windowRect.left, windowRect.top).toVector2i();

    RECT clientRect{};
    REQUIRE(GetClientRect(handle, &clientRect));
    const auto initialSize = sf::Vector2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVector2u();
    constexpr sf::Vector2u newSize(640, 480);

    // Test sf::WindowBase functionality
    SUBCASE("sf::WindowBase tests")
    {
        sf::base::Optional<sf::WindowBase> windowBase;

        SUBCASE("WindowHandle constructor")
        {
            windowBase.emplace(handle);
        }

        INFO("sf::WindowBase test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(windowBase->getPosition() == position);
        CHECK(windowBase->getSize() == initialSize);
        CHECK(windowBase->getNativeHandle() == handle);

        CHECK(windowBase->getSize() != newSize);
        windowBase->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = sf::Vector2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVector2u();
        CHECK(size == newSize);
        CHECK(windowBase->getSize() == size);
    }

    // Test sf::Window functionality
    SUBCASE("sf::Window tests")
    {
        sf::base::Optional<sf::Window> window;

        SUBCASE("Default context settings")
        {
            SUBCASE("WindowHandle constructor")
            {
                window.emplace(handle);
            }
            INFO("sf::Window default context test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(window->getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SUBCASE("Custom context settings")
        {
            static constexpr sf::ContextSettings contextSettings{1, 1, 1};
            SUBCASE("WindowHandle constructor")
            {
                window.emplace(handle, contextSettings);
            }
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
        const auto size = sf::Vector2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVector2u();
        CHECK(size == newSize);
        CHECK(window->getSize() == size);
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
