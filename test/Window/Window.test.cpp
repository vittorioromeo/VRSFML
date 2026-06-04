#include "Zancle/Window/Window.hpp"

#include "Zancle/Window/WindowContext.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowSettings.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

// Other 1st party headers
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Window] za::Window" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_HAS_VIRTUAL_DESTRUCTOR(za::Window));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::Window));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::Window));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Window));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Window));
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            const auto window = za::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, and style constructor")
        {
            const auto window = za::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const auto window = za::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, state, and context settings constructor")
        {
            const auto window = za::Window::create({.size{360u, 240u},
                                                    .title = "Window Tests",
                                                    .contextSettings{.depthBits = 1, .stencilBits = 1}})
                                    .value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
        }

        SECTION("Mode, title, and state")
        {
            const auto window = za::Window::create({.size{360u, 240u}, .title = "Window Tests"}).value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().attributeFlags == za::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, state, and context settings constructor")
        {
            const auto window = za::Window::create({.size{360u, 240u},
                                                    .title = "Window Tests",
                                                    .contextSettings{.depthBits = 1, .stencilBits = 1}})
                                    .value();

            CHECK(window.getSize() == za::Vec2u{360, 240});
            CHECK(window.getNativeHandle() != za::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
        }
    }

// Creating multiple windows in Emscripten is not supported
#ifndef ZA_SYSTEM_EMSCRIPTEN
    SECTION("Multiple windows display orderings")
    {
        auto window      = za::Window::create({.size{256u, 256u}, .title = "A"}).value();
        auto childWindow = za::Window::create(za::WindowSettings{.size{256u, 256u}, .title = "B"});

        // Variant 1: parent display, child reset, parent display
        window.display();
        childWindow->display();
        window.display();
        childWindow->display();
        window.display();
        childWindow.reset();
        window.display();
    }
#endif
}
