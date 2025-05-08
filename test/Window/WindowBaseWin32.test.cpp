#include "SFML/Config.hpp"
#ifdef SFML_SYSTEM_WINDOWS

    #include "SFML/Window/WindowBase.hpp"
    #include "SFML/Window/WindowContext.hpp"

    #include "SFML/Base/Assert.hpp"
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
bool gotWmShowWindow0 = false;

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

        m_handle = CreateWindowW(reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(m_winClassId)),
                                 L"WindowBase Tests",
                                 WS_OVERLAPPEDWINDOW,
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
        gotWmShowWindow0 = false;
    }

    [[nodiscard]] HWND getHandle() const
    {
        return m_handle;
    }

private:
    static LRESULT WINAPI wndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_SHOWWINDOW:
                gotWmShowWindow0 = true;
                SFML_BASE_ASSERT(wParam == TRUE); // If wParam is TRUE, the window is being shown
                SFML_BASE_ASSERT(
                    lParam == 0); // If lParam is zero, the message was sent because of a call to the ShowWindow function
                break;
            case WM_CLOSE:
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
        windowBase.emplace(handle);
    }

    RECT rect{};
    GetClientRect(handle, &rect);
    CHECK(windowBase->getSize() == sf::Vec2(rect.right - rect.left, rect.bottom - rect.top).toVec2u());
    CHECK(windowBase->getNativeHandle() == handle);

    CHECK(gotWmShowWindow0);
    CHECK(IsWindow(handle));

    windowBase.reset();
    CHECK(IsWindow(handle)); // The window is not destroyed.

    nativeWindow.reset();
    CHECK(!IsWindow(handle)); // Now it is gone
}

#endif
