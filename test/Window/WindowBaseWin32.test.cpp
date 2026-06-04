#include "SFML/Config.hpp" // IWYU pragma: keep

#ifdef SFML_SYSTEM_WINDOWS

    #include "SystemUtil.hpp"
    #include "Tst/Tst.hpp"

    #include "SFML/Window/WindowBase.hpp"
    #include "SFML/Window/WindowContext.hpp"

    #include "SFML/System/WindowsHeader.hpp" // IWYU pragma: keep

    #include "SFML/Base/Assert.hpp"
    #include "SFML/Base/Optional.hpp"


namespace
{
class NativeWindow
{
public:
    NativeWindow()
    {
        m_classInfo.lpfnWndProc   = wndProc;
        m_classInfo.hInstance     = GetModuleHandleW(nullptr);
        m_classInfo.lpszClassName = L"sfml_WindowBaseTests";

        m_winClassId = RegisterClassW(&m_classInfo);
        SFML_BASE_ASSERT(m_winClassId);

        // Create the window already visible. The SDL-backed WindowBase wraps a
        // borrowed handle without altering its visibility, so the test exercises
        // takeover of an already-shown window.
        m_handle = CreateWindowW(reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(m_winClassId)),
                                 L"WindowBase Tests",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 640,
                                 480,
                                 nullptr,
                                 nullptr,
                                 m_classInfo.hInstance,
                                 nullptr);
        SFML_BASE_ASSERT(m_handle);
    }

    ~NativeWindow()
    {
        DestroyWindow(m_handle);
        UnregisterClassW(m_classInfo.lpszClassName, m_classInfo.hInstance);
    }

    [[nodiscard]] HWND getHandle() const
    {
        return m_handle;
    }

private:
    static LRESULT WINAPI wndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CLOSE)
        {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(handle, message, wParam, lParam);
    }

    WNDCLASSW m_classInfo{};
    ATOM      m_winClassId{};
    HWND      m_handle{};
};
} // namespace

TEST_CASE("[Window] sf::WindowBase (Win32)")
{
    auto windowContext = sf::WindowContext::create().value();

    sf::base::Optional<NativeWindow>   nativeWindow(sf::base::inPlace);
    sf::base::Optional<sf::WindowBase> windowBase;

    const HWND handle = nativeWindow->getHandle();

    SECTION("WindowHandle constructor")
    {
        windowBase = sf::WindowBase::create(handle);
        CHECK(windowBase.hasValue());
    }

    RECT rect{};
    GetClientRect(handle, &rect);
    CHECK(windowBase->getSize() == sf::Vec2(rect.right - rect.left, rect.bottom - rect.top).toVec2u());
    CHECK(windowBase->getNativeHandle() == handle);

    CHECK(IsWindow(handle));

    windowBase.reset();
    CHECK(IsWindow(handle)); // The window is not destroyed.

    nativeWindow.reset();
    CHECK(!IsWindow(handle)); // Now it is gone
}

#endif
