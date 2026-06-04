#include "Zancle/Config.hpp" // IWYU pragma: keep

#ifdef ZA_SYSTEM_WINDOWS
// Other 1st party headers
    #include "SystemUtil.hpp"
    #include "Tst/Tst.hpp"

    #include "Zancle/Graphics/RenderWindow.hpp"

    #include "Zancle/Window/Window.hpp"
    #include "Zancle/Window/WindowBase.hpp"
    #include "Zancle/Window/WindowContext.hpp"
    #include "Zancle/Window/WindowHandle.hpp"

    #include "Zancle/System/Priv/Vec2Base.hpp"
    #include "Zancle/System/WindowsHeader.hpp" // IWYU pragma: keep

    #include "ZancleBase/Optional.hpp"


namespace
{
LRESULT WINAPI wndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CLOSE)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(handle, message, wParam, lParam);
}

// Create the window already visible. The SDL-backed WindowBase/Window/RenderWindow
// wrap a borrowed handle without altering its visibility, so the test exercises
// takeover of an already-shown window.
za::WindowHandle createWindowWrapper(LPWSTR className, HINSTANCE hInstance, DWORD dwExStyle, bool withMenu)
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
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
    const za::WindowHandle handle = createWindowWrapper(reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(winClassId)),
                                                        classInfo.hInstance,
                                                        exStyle,
                                                        withMenu);
    REQUIRE(handle);
    REQUIRE(IsWindow(handle));

    RECT windowRect{};
    REQUIRE(GetClientRect(handle, &windowRect));
    ClientToScreen(handle, reinterpret_cast<LPPOINT>(&windowRect));
    const auto position = za::Vec2(windowRect.left, windowRect.top).toVec2i();

    RECT clientRect{};
    REQUIRE(GetClientRect(handle, &clientRect));
    const auto initialSize = za::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
    constexpr za::Vec2u newSize(640, 480);

    // Test za::WindowBase functionality
    SUBCASE("za::WindowBase tests")
    {
        zb::Optional<za::WindowBase> windowBase;

        windowBase = za::WindowBase::create(handle);
        CHECK(windowBase.hasValue());

        INFO("za::WindowBase test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(windowBase->getPosition() == position);
        CHECK(windowBase->getSize() == initialSize);
        CHECK(windowBase->getNativeHandle() == handle);

        CHECK(windowBase->getSize() != newSize);
        windowBase->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = za::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);
        CHECK(windowBase->getSize() == size);
    }

    // Test za::Window functionality
    SUBCASE("za::Window tests")
    {
        zb::Optional<za::Window> window;

        SUBCASE("Default context settings")
        {
            window = za::Window::create(handle);
            REQUIRE(window.hasValue());

            INFO("za::Window default context test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(window->getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SUBCASE("Custom context settings")
        {
            static constexpr za::ContextSettings contextSettings{.depthBits = 1, .stencilBits = 1, .majorVersion = 1};

            window = za::Window::create(handle, contextSettings);
            REQUIRE(window.hasValue());

            INFO("za::Window custom context test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(window->getSettings().depthBits >= 1);
            CHECK(window->getSettings().stencilBits >= 1);
        }

        REQUIRE(window.hasValue());
        INFO("za::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(window->getPosition() == position);
        CHECK(window->getSize() == initialSize);
        CHECK(window->getNativeHandle() == handle);

        CHECK(window->getSize() != newSize);
        window->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = za::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);
        CHECK(window->getSize() == size);
    }

    SECTION("za::RenderWindow")
    {
        zb::Optional<za::RenderWindow> renderWindow;

        SECTION("Default context settings")
        {
            renderWindow = za::RenderWindow::create(handle);
            REQUIRE(renderWindow.hasValue());

            INFO("za::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(renderWindow->getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SECTION("Custom context settings")
        {
            static constexpr za::ContextSettings contextSettings{.depthBits = 1, .stencilBits = 1};

            renderWindow = za::RenderWindow::create(handle, contextSettings);
            REQUIRE(renderWindow.hasValue());

            INFO("za::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
            CHECK(renderWindow->getSettings().depthBits >= 1);
            CHECK(renderWindow->getSettings().stencilBits >= 1);
        }

        REQUIRE(renderWindow.hasValue());
        INFO("za::Window test with exStyle: " << exStyle << ", withMenu: " << withMenu);
        CHECK(renderWindow->getPosition() == position);
        CHECK(renderWindow->getSize() == initialSize);
        CHECK(renderWindow->getNativeHandle() == handle);

        CHECK(renderWindow->getSize() != newSize);
        renderWindow->setSize(newSize);

        REQUIRE(GetClientRect(handle, &clientRect));
        const auto size = za::Vec2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top).toVec2u();
        CHECK(size == newSize);                 // Validate that the actual client rect is indeed what we asked for
        CHECK(renderWindow->getSize() == size); // Validate that the `getSize` also returns the _actual_ client size
    }

    INFO("Final checks with exStyle: " << exStyle << ", withMenu: " << withMenu);
    CHECK(IsWindow(handle));

    CHECK(DestroyWindow(handle));
    CHECK(UnregisterClassW(classInfo.lpszClassName, classInfo.hInstance));
    CHECK(!IsWindow(handle));
}
} // anonymous namespace

TEST_CASE("[Window] za::WindowHandle (Win32) - Parameterized")
{
    auto windowContext = za::WindowContext::create().value();

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
