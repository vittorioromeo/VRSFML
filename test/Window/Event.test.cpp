#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Window/Event.hpp"

#include "Zancle/Window/Joystick.hpp"
#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"
#include "Zancle/Window/Sensor.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Vec3.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsEmpty.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


namespace
{
////////////////////////////////////////////////////////////
struct
{
    int operator()(za::Event::Closed&) const
    {
        return 0;
    }

    int operator()(const za::Event::Closed&) const
    {
        return 1;
    }

    int operator()(za::Event::Resized&) const
    {
        return 2;
    }

    int operator()(za::Event::KeyPressed) const
    {
        return 3;
    }

    int operator()(za::Event::FocusGained) const
    {
        return 4;
    }

    template <typename T>
    int operator()(T) const
    {
        return 5;
    }
} visitor;

} // namespace

TEST_CASE("[Window] za::Event")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_DEFAULT_CONSTRUCTIBLE(za::Event));
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::Event));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::Event));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Event));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Event));

        STATIC_CHECK(!ZB_IS_TRIVIAL(za::Event)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::Event));
        STATIC_CHECK(!ZB_IS_AGGREGATE(za::Event));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::Event));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::Event));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::Event, za::Event));
    }

    SECTION("Construction")
    {
        SECTION("Template constructor")
        {
            const za::Event event = za::Event::Resized{{1, 2}};
            CHECK(event.is<za::Event::Resized>());
            CHECK(event.getIf<za::Event::Resized>());
            const auto& resized = *event.getIf<za::Event::Resized>();
            CHECK(resized.size == za::Vec2u{1, 2});
        }
    }

    SECTION("Assign all possible values")
    {
        za::Event event = za::Event::Closed{};
        CHECK(event.is<za::Event::Closed>());
        CHECK(event.getIf<za::Event::Closed>());

        event = za::Event::Resized{{1, 2}};
        CHECK(event.is<za::Event::Resized>());
        CHECK(event.getIf<za::Event::Resized>());
        const auto& resized = *event.getIf<za::Event::Resized>();
        CHECK(resized.size == za::Vec2u{1, 2});

        event = za::Event::FocusLost{};
        CHECK(event.is<za::Event::FocusLost>());
        CHECK(event.getIf<za::Event::FocusLost>());

        event = za::Event::FocusGained{};
        CHECK(event.is<za::Event::FocusGained>());
        CHECK(event.getIf<za::Event::FocusGained>());

        event = za::Event::TextEntered{123'456};
        CHECK(event.is<za::Event::TextEntered>());
        CHECK(event.getIf<za::Event::TextEntered>());
        const auto& textEntered = *event.getIf<za::Event::TextEntered>();
        CHECK(textEntered.unicode == 123'456);

        event = za::Event::KeyPressed{za::Keyboard::Key::C, za::Keyboard::Scan::C, true, true, true, true, true, true, true};
        CHECK(event.is<za::Event::KeyPressed>());
        CHECK(event.getIf<za::Event::KeyPressed>());
        const auto& keyPressed = *event.getIf<za::Event::KeyPressed>();
        CHECK(keyPressed.code == za::Keyboard::Key::C);
        CHECK(keyPressed.scancode == za::Keyboard::Scan::C);
        CHECK(keyPressed.alt);
        CHECK(keyPressed.control);
        CHECK(keyPressed.shift);
        CHECK(keyPressed.system);
        CHECK(keyPressed.capsLock);
        CHECK(keyPressed.numLock);
        CHECK(keyPressed.scrollLock);

        event = za::Event::KeyReleased{za::Keyboard::Key::D, za::Keyboard::Scan::D, true, true, true, true, true, true, true};
        CHECK(event.is<za::Event::KeyReleased>());
        CHECK(event.getIf<za::Event::KeyReleased>());
        const auto& keyReleased = *event.getIf<za::Event::KeyReleased>();
        CHECK(keyReleased.code == za::Keyboard::Key::D);
        CHECK(keyReleased.scancode == za::Keyboard::Scan::D);
        CHECK(keyReleased.alt);
        CHECK(keyReleased.control);
        CHECK(keyReleased.shift);
        CHECK(keyReleased.system);
        CHECK(keyReleased.capsLock);
        CHECK(keyReleased.numLock);
        CHECK(keyReleased.scrollLock);

        event = za::Event::MouseWheelScrolled{za::Mouse::Wheel::Horizontal, 3.14f, {4, 5}};
        CHECK(event.is<za::Event::MouseWheelScrolled>());
        CHECK(event.getIf<za::Event::MouseWheelScrolled>());
        const auto& mouseWheelScrolled = *event.getIf<za::Event::MouseWheelScrolled>();
        CHECK(mouseWheelScrolled.wheel == za::Mouse::Wheel::Horizontal);
        CHECK(mouseWheelScrolled.delta == 3.14f);
        CHECK(mouseWheelScrolled.position == za::Vec2i{4, 5});

        event = za::Event::MouseButtonPressed{za::Mouse::Button::Middle, {6, 7}};
        CHECK(event.is<za::Event::MouseButtonPressed>());
        CHECK(event.getIf<za::Event::MouseButtonPressed>());
        const auto& mouseButtonPressed = *event.getIf<za::Event::MouseButtonPressed>();
        CHECK(mouseButtonPressed.button == za::Mouse::Button::Middle);
        CHECK(mouseButtonPressed.position == za::Vec2i{6, 7});

        event = za::Event::MouseButtonReleased{za::Mouse::Button::Extra1, {8, 9}};
        CHECK(event.is<za::Event::MouseButtonReleased>());
        CHECK(event.getIf<za::Event::MouseButtonReleased>());
        const auto& mouseButtonReleased = *event.getIf<za::Event::MouseButtonReleased>();
        CHECK(mouseButtonReleased.button == za::Mouse::Button::Extra1);
        CHECK(mouseButtonReleased.position == za::Vec2i{8, 9});

        event = za::Event::MouseMoved{{4, 2}};
        CHECK(event.is<za::Event::MouseMoved>());
        CHECK(event.getIf<za::Event::MouseMoved>());
        const auto& mouseMoved = *event.getIf<za::Event::MouseMoved>();
        CHECK(mouseMoved.position == za::Vec2i{4, 2});

        event = za::Event::MouseMovedRaw{{3, 7}};
        CHECK(event.is<za::Event::MouseMovedRaw>());
        CHECK(event.getIf<za::Event::MouseMovedRaw>());
        const auto& mouseMovedRaw = *event.getIf<za::Event::MouseMovedRaw>();
        CHECK(mouseMovedRaw.delta == za::Vec2i{3, 7});

        event = za::Event::MouseEntered{};
        CHECK(event.is<za::Event::MouseEntered>());
        CHECK(event.getIf<za::Event::MouseEntered>());

        event = za::Event::MouseLeft{};
        CHECK(event.is<za::Event::MouseLeft>());
        CHECK(event.getIf<za::Event::MouseLeft>());

        event = za::Event::JoystickButtonPressed{100, 200};
        CHECK(event.is<za::Event::JoystickButtonPressed>());
        CHECK(event.getIf<za::Event::JoystickButtonPressed>());
        const auto& joystickButtonPressed = *event.getIf<za::Event::JoystickButtonPressed>();
        CHECK(joystickButtonPressed.joystickId == 100);
        CHECK(joystickButtonPressed.button == 200);

        event = za::Event::JoystickButtonReleased{300, 400};
        CHECK(event.is<za::Event::JoystickButtonReleased>());
        CHECK(event.getIf<za::Event::JoystickButtonReleased>());
        const auto& joystickButtonReleased = *event.getIf<za::Event::JoystickButtonReleased>();
        CHECK(joystickButtonReleased.joystickId == 300);
        CHECK(joystickButtonReleased.button == 400);

        event = za::Event::JoystickMoved{300, za::Joystick::Axis::Z, 1.23f};
        CHECK(event.is<za::Event::JoystickMoved>());
        CHECK(event.getIf<za::Event::JoystickMoved>());
        const auto& joystickMoved = *event.getIf<za::Event::JoystickMoved>();
        CHECK(joystickMoved.joystickId == 300);
        CHECK(joystickMoved.axis == za::Joystick::Axis::Z);
        CHECK(joystickMoved.position == 1.23f);

        event = za::Event::JoystickConnected{42};
        CHECK(event.is<za::Event::JoystickConnected>());
        CHECK(event.getIf<za::Event::JoystickConnected>());
        const auto& joystickConnected = *event.getIf<za::Event::JoystickConnected>();
        CHECK(joystickConnected.joystickId == 42);

        event = za::Event::JoystickDisconnected{43};
        CHECK(event.is<za::Event::JoystickDisconnected>());
        CHECK(event.getIf<za::Event::JoystickDisconnected>());
        const auto& joystickDisconnected = *event.getIf<za::Event::JoystickDisconnected>();
        CHECK(joystickDisconnected.joystickId == 43);

        event = za::Event::TouchBegan{99, {98, 97}, 0.f};
        CHECK(event.is<za::Event::TouchBegan>());
        CHECK(event.getIf<za::Event::TouchBegan>());
        const auto& touchBegan = *event.getIf<za::Event::TouchBegan>();
        CHECK(touchBegan.finger == 99);
        CHECK(touchBegan.position == za::Vec2i{98, 97});

        event = za::Event::TouchMoved{96, {95, 94}, 0.f};
        CHECK(event.is<za::Event::TouchMoved>());
        CHECK(event.getIf<za::Event::TouchMoved>());
        const auto& touchMoved = *event.getIf<za::Event::TouchMoved>();
        CHECK(touchMoved.finger == 96);
        CHECK(touchMoved.position == za::Vec2i{95, 94});

        event = za::Event::TouchEnded{93, {92, 91}, 0.f};
        CHECK(event.is<za::Event::TouchEnded>());
        CHECK(event.getIf<za::Event::TouchEnded>());
        const auto& touchEnded = *event.getIf<za::Event::TouchEnded>();
        CHECK(touchEnded.finger == 93);
        CHECK(touchEnded.position == za::Vec2i{92, 91});

        event = za::Event::SensorChanged{za::Sensor::Type::Gravity, {1.2f, 3.4f, 5.6f}};
        CHECK(event.is<za::Event::SensorChanged>());
        CHECK(event.getIf<za::Event::SensorChanged>());
        const auto& sensorChanged = *event.getIf<za::Event::SensorChanged>();
        CHECK(sensorChanged.type == za::Sensor::Type::Gravity);
        CHECK(sensorChanged.value == za::Vec3f(1.2f, 3.4f, 5.6f));
    }

    SECTION("Subtypes")
    {
        // Empty structs
        STATIC_CHECK(ZB_IS_EMPTY(za::Event::Closed));
        STATIC_CHECK(ZB_IS_EMPTY(za::Event::FocusLost));
        STATIC_CHECK(ZB_IS_EMPTY(za::Event::FocusGained));
        STATIC_CHECK(ZB_IS_EMPTY(za::Event::MouseEntered));
        STATIC_CHECK(ZB_IS_EMPTY(za::Event::MouseLeft));

        // Non-empty structs
        const za::Event::Resized resized{};
        CHECK(resized.size == za::Vec2u{});

        const za::Event::TextEntered textEntered;
        CHECK(textEntered.unicode == 0);

        const za::Event::KeyPressed keyPressed;
        CHECK(keyPressed.code == za::Keyboard::Key{});
        CHECK(keyPressed.scancode == za::Keyboard::Scancode{});
        CHECK(!keyPressed.alt);
        CHECK(!keyPressed.control);
        CHECK(!keyPressed.shift);
        CHECK(!keyPressed.system);

        const za::Event::KeyReleased keyReleased;
        CHECK(keyReleased.code == za::Keyboard::Key{});
        CHECK(keyReleased.scancode == za::Keyboard::Scancode{});
        CHECK(!keyReleased.alt);
        CHECK(!keyReleased.control);
        CHECK(!keyReleased.shift);
        CHECK(!keyReleased.system);

        const za::Event::MouseWheelScrolled mouseWheelScrolled{};
        CHECK(mouseWheelScrolled.wheel == za::Mouse::Wheel{});
        CHECK(mouseWheelScrolled.delta == 0);
        CHECK(mouseWheelScrolled.position == za::Vec2i{});

        const za::Event::MouseButtonPressed mouseButtonPressed{};
        CHECK(mouseButtonPressed.button == za::Mouse::Button{});
        CHECK(mouseButtonPressed.position == za::Vec2i{});

        const za::Event::MouseButtonReleased mouseButtonReleased{};
        CHECK(mouseButtonReleased.button == za::Mouse::Button{});
        CHECK(mouseButtonReleased.position == za::Vec2i{});

        const za::Event::MouseMoved mouseMoved{};
        CHECK(mouseMoved.position == za::Vec2i{});

        const za::Event::MouseMovedRaw mouseMovedRaw{};
        CHECK(mouseMovedRaw.delta == za::Vec2i{});

        const za::Event::JoystickButtonPressed joystickButtonPressed;
        CHECK(joystickButtonPressed.joystickId == 0);
        CHECK(joystickButtonPressed.button == 0);

        const za::Event::JoystickButtonReleased joystickButtonReleased;
        CHECK(joystickButtonReleased.joystickId == 0);
        CHECK(joystickButtonReleased.button == 0);

        const za::Event::JoystickMoved joystickMoved;
        CHECK(joystickMoved.joystickId == 0);
        CHECK(joystickMoved.axis == za::Joystick::Axis{});
        CHECK(joystickMoved.position == 0);

        const za::Event::JoystickConnected joystickConnected;
        CHECK(joystickConnected.joystickId == 0);

        const za::Event::JoystickDisconnected joystickDisconnected;
        CHECK(joystickDisconnected.joystickId == 0);

        const za::Event::TouchBegan touchBegan{};
        CHECK(touchBegan.finger == 0);
        CHECK(touchBegan.position == za::Vec2i{});

        const za::Event::TouchMoved touchMoved{};
        CHECK(touchMoved.finger == 0);
        CHECK(touchMoved.position == za::Vec2i{});

        const za::Event::TouchEnded touchEnded{};
        CHECK(touchEnded.finger == 0);
        CHECK(touchEnded.position == za::Vec2i{});

        const za::Event::SensorChanged sensorChanged{};
        CHECK(sensorChanged.type == za::Sensor::Type{});
        CHECK(sensorChanged.value == za::Vec3f());
    }

    SECTION("getIf()")
    {
        za::Event event      = za::Event::MouseMoved{{4, 2}};
        auto*     mouseMoved = event.getIf<za::Event::MouseMoved>();
        REQUIRE(mouseMoved);
        mouseMoved->position = za::Vec2i(6, 9);
        CHECK(mouseMoved->position == za::Vec2i(6, 9));
    }

    SECTION("visit()")
    {
        SECTION("Non-const")
        {
            za::Event closed = za::Event::Closed{};
            CHECK(closed.visit(visitor) == 0);

            za::Event resized = za::Event::Resized{};
            CHECK(resized.visit(visitor) == 2);

            za::Event keyPressed = za::Event::KeyPressed{};
            CHECK(keyPressed.visit(visitor) == 3);

            za::Event focusLost = za::Event::FocusLost{};
            CHECK(focusLost.visit(visitor) == 5);
        }

        SECTION("Const")
        {
            const za::Event closed = za::Event::Closed{};
            CHECK(closed.visit(visitor) == 1);

            const za::Event resized = za::Event::Resized{};
            CHECK(resized.visit(visitor) == 5); // Cannot use non-const reference callback

            const za::Event keyPressed = za::Event::KeyPressed{};
            CHECK(keyPressed.visit(visitor) == 3);

            const za::Event focusLost = za::Event::FocusLost{};
            CHECK(focusLost.visit(visitor) == 5);
        }

        SECTION("Move-only visitor")
        {
            struct MoveOnly
            {
                MoveOnly()                = default;
                MoveOnly(const MoveOnly&) = delete;
                MoveOnly(MoveOnly&&)      = default;
            };

            auto moveOnlyVisitor = [p = MoveOnly{}](const auto&)
            {
                (void)p;
                return 100;
            };

            const za::Event closed = za::Event::Closed{};
            CHECK(closed.visit(moveOnlyVisitor) == 100);
        }
    }
}
