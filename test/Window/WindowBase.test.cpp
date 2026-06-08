#include "Zancle/Window/WindowBase.hpp"

// Other 1st party headers
#include "StringifyOptionalUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/WindowContext.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "Zancle/Chrono/Clock.hpp"
#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


namespace
{

template <typename T>
constexpr const T& asConst(T& t) noexcept
{
    return t;
}

} // namespace


TEST_CASE("[Window] za::WindowBase" * tst::skip(skipDisplayTests))
{
    [[maybe_unused]] auto& windowContext = TST_CASE_SHARED(za::WindowContext::create().value());

    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_HAS_VIRTUAL_DESTRUCTOR(za::WindowBase));
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::WindowBase));
        STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(za::WindowBase));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::WindowBase));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::WindowBase));
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            {
                const auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = ""}).value();

                CHECK(windowBase.getSize() == za::Vec2u{360, 240});
                CHECK(windowBase.getNativeHandle() != za::WindowHandle());
            }
            {
                const auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

                CHECK(windowBase.getSize() == za::Vec2u{360, 240});
                CHECK(windowBase.getNativeHandle() != za::WindowHandle());
            }
        }

        SECTION("Mode, title, and style constructor")
        {
            const auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            CHECK(windowBase.getSize() == za::Vec2u{360, 240});
            CHECK(windowBase.getNativeHandle() != za::WindowHandle());
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            CHECK(windowBase.getSize() == za::Vec2u{360, 240});
            CHECK(windowBase.getNativeHandle() != za::WindowHandle());
        }

        SECTION("Mode, title, and state constructor")
        {
            const auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            CHECK(windowBase.getSize() == za::Vec2u{360, 240});
            CHECK(windowBase.getNativeHandle() != za::WindowHandle());
        }
    }

    SECTION("waitEvent()")
    {
        SECTION("Initialized window")
        {
            auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            constexpr auto timeout = za::milliseconds(10);

            za::Clock clock;

            const auto startTime = clock.getElapsedTime();
            const auto event     = windowBase.waitEvent(timeout);
            const auto elapsed   = clock.getElapsedTime() - startTime;

            REQUIRE(elapsed < (timeout + za::milliseconds(100)));

            if (elapsed <= timeout)
                CHECK(event.hasValue());
            else
                CHECK(!event.hasValue());
        }
    }

    SECTION("Set/get position")
    {
        auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

        windowBase.setPosition({12, 34});

        // TODO P1: unreliable, depends on window manager behavior
        // CHECK(windowBase.getPosition() == za::Vec2i{12, 34});
    }

    SECTION("Set/get size")
    {
        SECTION("Initialized window")
        {
            auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            windowBase.setSize({128, 256});
            CHECK(windowBase.getSize() == za::Vec2u{128, 256});
        }

        SECTION("Minimum size")
        {
            auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            windowBase.setMinimumSize({128u, 256u});
            windowBase.setSize({100, 100});
            CHECK(windowBase.getSize() == za::Vec2u{128, 256});
        }

        SECTION("Maximum size")
        {
            auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

            windowBase.setMaximumSize({128u, 256u});
            windowBase.setSize({400, 400});
            CHECK(windowBase.getSize() == za::Vec2u{128, 256});
        }
    }

    SECTION("setMinimumSize()")
    {
        auto windowBase = za::WindowBase::create({.size{100u, 100u}, .title = "WindowBase Tests"}).value();

        windowBase.setMinimumSize({200u, 300u});
        CHECK(windowBase.getSize() == za::Vec2u{200, 300});
        windowBase.setMaximumSize({200u, 300u});
    }

    SECTION("setMinimumSize()")
    {
        auto windowBase = za::WindowBase::create({.size{400u, 400u}, .title = "WindowBase Tests"}).value();

        windowBase.setMaximumSize({200u, 300u});
        CHECK(windowBase.getSize() == za::Vec2u{200, 300});
        windowBase.setMinimumSize({200u, 300u});
    }

    // Test for compilation but do not run. This code sometimes hangs indefinitely
    // when running on the BuildBot CI pipeline. Because it contains no
    // assertions we have nothing to gain by running it anyways
    (void)[]
    {
        auto windowBase = za::WindowBase::create({.size{360u, 240u}, .title = "WindowBase Tests"}).value();

        // Should compile if user provides only a specific handler
        windowBase.pollAndHandleEvents([](za::Event::Closed) {});

        // Should compile if user provides only a catch-all
        windowBase.pollAndHandleEvents([](const auto&) {});

        // Should compile if user provides both a specific handler and a catch-all
        windowBase.pollAndHandleEvents([](za::Event::Closed) {}, [](const auto&) {});
        windowBase.pollAndHandleEvents([](const za::Event::Closed&) {}, [](const auto&) {});

        // Should compile if user provides a handler taking an event subtype by value or reference,
        // but not rvalue reference because it would never be called.
        windowBase.pollAndHandleEvents([](za::Event::Closed) {});
        windowBase.pollAndHandleEvents([](const za::Event::Closed) {});
        windowBase.pollAndHandleEvents([](za::Event::Closed&) {});
        windowBase.pollAndHandleEvents([](const za::Event::Closed&) {});

        // Should compile if user provides a move-only handler
        struct MoveOnly
        {
            MoveOnly()                = default;
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&&)      = default;
        };

        windowBase.pollAndHandleEvents([p = MoveOnly{}](const za::Event::Closed&) { (void)p; });

        // Should compile if user provides a handler with deleted rvalue ref-qualified call operator
        struct LvalueOnlyHandler
        {
            void operator()(const za::Event::Closed&) &
            {
            }

            void operator()(const za::Event::Closed&) && = delete;
        };

        windowBase.pollAndHandleEvents(LvalueOnlyHandler{});

        // Should compile if user provides a reference to a handler
        auto handler = [](const za::Event::Closed&) {};
        windowBase.pollAndHandleEvents(handler);
        windowBase.pollAndHandleEvents(asConst(handler));

        // Should compile if user provides a function pointer
        windowBase.pollAndHandleEvents(+[](const za::Event::Closed&) {});
        // clang-format off
    };
    // clang-format on
}
