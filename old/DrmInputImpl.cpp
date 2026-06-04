#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/InputImpl.hpp"

#include "Zancle/System/Err.hpp"

#include "ZancleBase/Algorithm.hpp"
#include "ZancleBase/Builtins/Memcpy.hpp"
#include "ZancleBase/EnumArray.hpp"
#include "ZancleBase/Optional.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <mutex>
#include <queue>
#include <sstream>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include <cerrno>
#include <cstdio>
#include <cstdlib>


namespace
{
struct TouchSlot
{
    base::Optional<unsigned int> oldId;
    base::Optional<unsigned int> id;
    za::Vector2i                 pos;
};

std::recursive_mutex inputMutex; // threadsafe? maybe...
za::Vector2i         mousePos;   // current mouse position

std::vector<int> fileDescriptors; // list of open file descriptors for /dev/input
zb::EnumArray<za::Mouse::Button, bool, za::Mouse::ButtonCount> mouseMap{}; // track whether mouse buttons are down
zb::EnumArray<za::Keyboard::Key, bool, za::Keyboard::KeyCount> keyMap{};   // track whether keys are down

int                    touchFd = -1;    // file descriptor we have seen MT events on; assumes only 1
std::vector<TouchSlot> touchSlots;      // track the state of each touch "slot"
int                    currentSlot = 0; // which slot are we currently updating?

std::queue<za::Event> eventQueue;    // events received and waiting to be consumed
constexpr int         maxQueue = 64; // The maximum size we let eventQueue grow to

termios newTerminalConfig, oldTerminalConfig; // Terminal configurations

bool altDown()
{
    return keyMap[za::Keyboard::Key::LAlt] || keyMap[za::Keyboard::Key::RAlt];
}

bool controlDown()
{
    return keyMap[za::Keyboard::Key::LControl] || keyMap[za::Keyboard::Key::RControl];
}

bool shiftDown()
{
    return keyMap[za::Keyboard::Key::LShift] || keyMap[za::Keyboard::Key::RShift];
}

bool systemDown()
{
    return keyMap[za::Keyboard::Key::LSystem] || keyMap[za::Keyboard::Key::RSystem];
}

void uninitFileDescriptors()
{
    for (const auto& fileDescriptor : fileDescriptors)
        close(fileDescriptor);
}

#define BITS_PER_LONG        (sizeof(unsigned long) * 8)
#define NBITS(x)             ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)               ((x) % BITS_PER_LONG)
#define LONG(x)              ((x) / BITS_PER_LONG)
#define TEST_BIT(bit, array) (((array)[LONG(bit)] >> OFF(bit)) & 1)

// Only keep fileDescriptors that we think are a keyboard, mouse or touchpad/touchscreen
// Joysticks are handled in /src/SFML/Window/Unix/JoystickImpl.cpp
bool keepFileDescriptor(int fileDesc)
{
    unsigned long bitmaskEv[NBITS(EV_MAX)];
    unsigned long bitmaskKey[NBITS(KEY_MAX)];
    unsigned long bitmaskAbs[NBITS(ABS_MAX)];
    unsigned long bitmaskRel[NBITS(REL_MAX)];

    ioctl(fileDesc, EVIOCGBIT(0, sizeof(bitmaskEv)), &bitmaskEv);
    ioctl(fileDesc, EVIOCGBIT(EV_KEY, sizeof(bitmaskKey)), &bitmaskKey);
    ioctl(fileDesc, EVIOCGBIT(EV_ABS, sizeof(bitmaskAbs)), &bitmaskAbs);
    ioctl(fileDesc, EVIOCGBIT(EV_REL, sizeof(bitmaskRel)), &bitmaskRel);

    // This is the keyboard test used by SDL.
    // The first 32 bits are ESC, numbers and Q to D;  If we have any of those,
    // consider it a keyboard device; do not test for KEY_RESERVED, though
    const bool isKeyboard = (bitmaskKey[0] & 0xFF'FF'FF'FE);

    const bool isAbs = TEST_BIT(EV_ABS, bitmaskEv) && TEST_BIT(ABS_X, bitmaskAbs) && TEST_BIT(ABS_Y, bitmaskAbs);

    const bool isRel = TEST_BIT(EV_REL, bitmaskEv) && TEST_BIT(REL_X, bitmaskRel) && TEST_BIT(REL_Y, bitmaskRel);

    const bool isMouse = (isAbs || isRel) && TEST_BIT(BTN_MOUSE, bitmaskKey);

    const bool isTouch = isAbs && (TEST_BIT(BTN_TOOL_FINGER, bitmaskKey) || TEST_BIT(BTN_TOUCH, bitmaskKey));

    return isKeyboard || isMouse || isTouch;
}

void initFileDescriptors()
{
    static bool initialized = false;
    if (initialized)
        return;

    initialized = true;

    for (int i = 0; i < 32; i++)
    {
        std::string        name("/dev/input/event");
        std::ostringstream stream;
        stream << i;
        name += stream.str();

        const int tempFD = open(name.c_str(), O_RDONLY | O_NONBLOCK);

        if (tempFD < 0)
        {
            if (errno != ENOENT)
                za::priv::err() << "Error opening " << name << ": " << std::strerror(errno);

            continue;
        }

        if (keepFileDescriptor(tempFD))
            fileDescriptors.push_back(tempFD);
        else
            close(tempFD);
    }

    std::atexit(uninitFileDescriptors);
}

base::Optional<za::Mouse::Button> toMouseButton(int code)
{
    switch (code)
    {
        case BTN_LEFT:
            return za::Mouse::Button::Left;
        case BTN_RIGHT:
            return za::Mouse::Button::Right;
        case BTN_MIDDLE:
            return za::Mouse::Button::Middle;
        case BTN_SIDE:
            return za::Mouse::Button::Extra1;
        case BTN_EXTRA:
            return za::Mouse::Button::Extra2;

        default:
            return base::nullOpt;
    }
}

za::Keyboard::Key toKey(int code)
{
    switch (code)
    {
            // clang-format off
        case KEY_ESC:           return za::Keyboard::Key::Escape;
        case KEY_1:             return za::Keyboard::Key::Num1;
        case KEY_2:             return za::Keyboard::Key::Num2;
        case KEY_3:             return za::Keyboard::Key::Num3;
        case KEY_4:             return za::Keyboard::Key::Num4;
        case KEY_5:             return za::Keyboard::Key::Num5;
        case KEY_6:             return za::Keyboard::Key::Num6;
        case KEY_7:             return za::Keyboard::Key::Num7;
        case KEY_8:             return za::Keyboard::Key::Num8;
        case KEY_9:             return za::Keyboard::Key::Num9;
        case KEY_0:             return za::Keyboard::Key::Num0;
        case KEY_MINUS:         return za::Keyboard::Key::Hyphen;
        case KEY_EQUAL:         return za::Keyboard::Key::Equal;
        case KEY_BACKSPACE:     return za::Keyboard::Key::Backspace;
        case KEY_TAB:           return za::Keyboard::Key::Tab;
        case KEY_Q:             return za::Keyboard::Key::Q;
        case KEY_W:             return za::Keyboard::Key::W;
        case KEY_E:             return za::Keyboard::Key::E;
        case KEY_R:             return za::Keyboard::Key::R;
        case KEY_T:             return za::Keyboard::Key::T;
        case KEY_Y:             return za::Keyboard::Key::Y;
        case KEY_U:             return za::Keyboard::Key::U;
        case KEY_I:             return za::Keyboard::Key::I;
        case KEY_O:             return za::Keyboard::Key::O;
        case KEY_P:             return za::Keyboard::Key::P;
        case KEY_LEFTBRACE:     return za::Keyboard::Key::LBracket;
        case KEY_RIGHTBRACE:    return za::Keyboard::Key::RBracket;
        case KEY_KPENTER:
        case KEY_ENTER:         return za::Keyboard::Key::Enter;
        case KEY_LEFTCTRL:      return za::Keyboard::Key::LControl;
        case KEY_A:             return za::Keyboard::Key::A;
        case KEY_S:             return za::Keyboard::Key::S;
        case KEY_D:             return za::Keyboard::Key::D;
        case KEY_F:             return za::Keyboard::Key::F;
        case KEY_G:             return za::Keyboard::Key::G;
        case KEY_H:             return za::Keyboard::Key::H;
        case KEY_J:             return za::Keyboard::Key::J;
        case KEY_K:             return za::Keyboard::Key::K;
        case KEY_L:             return za::Keyboard::Key::L;
        case KEY_SEMICOLON:     return za::Keyboard::Key::Semicolon;
        case KEY_APOSTROPHE:    return za::Keyboard::Key::Apostrophe;
        case KEY_GRAVE:         return za::Keyboard::Key::Grave;
        case KEY_LEFTSHIFT:     return za::Keyboard::Key::LShift;
        case KEY_BACKSLASH:     return za::Keyboard::Key::Backslash;
        case KEY_Z:             return za::Keyboard::Key::Z;
        case KEY_X:             return za::Keyboard::Key::X;
        case KEY_C:             return za::Keyboard::Key::C;
        case KEY_V:             return za::Keyboard::Key::V;
        case KEY_B:             return za::Keyboard::Key::B;
        case KEY_N:             return za::Keyboard::Key::N;
        case KEY_M:             return za::Keyboard::Key::M;
        case KEY_COMMA:         return za::Keyboard::Key::Comma;
        case KEY_DOT:           return za::Keyboard::Key::Period;
        case KEY_SLASH:         return za::Keyboard::Key::Slash;
        case KEY_RIGHTSHIFT:    return za::Keyboard::Key::RShift;
        case KEY_KPASTERISK:    return za::Keyboard::Key::Multiply;
        case KEY_LEFTALT:       return za::Keyboard::Key::LAlt;
        case KEY_SPACE:         return za::Keyboard::Key::Space;
        case KEY_F1:            return za::Keyboard::Key::F1;
        case KEY_F2:            return za::Keyboard::Key::F2;
        case KEY_F3:            return za::Keyboard::Key::F3;
        case KEY_F4:            return za::Keyboard::Key::F4;
        case KEY_F5:            return za::Keyboard::Key::F5;
        case KEY_F6:            return za::Keyboard::Key::F6;
        case KEY_F7:            return za::Keyboard::Key::F7;
        case KEY_F8:            return za::Keyboard::Key::F8;
        case KEY_F9:            return za::Keyboard::Key::F9;
        case KEY_F10:           return za::Keyboard::Key::F10;
        case KEY_F11:           return za::Keyboard::Key::F11;
        case KEY_F12:           return za::Keyboard::Key::F12;
        case KEY_F13:           return za::Keyboard::Key::F13;
        case KEY_F14:           return za::Keyboard::Key::F14;
        case KEY_F15:           return za::Keyboard::Key::F15;
        case KEY_KP7:           return za::Keyboard::Key::Numpad7;
        case KEY_KP8:           return za::Keyboard::Key::Numpad8;
        case KEY_KP9:           return za::Keyboard::Key::Numpad9;
        case KEY_KPMINUS:       return za::Keyboard::Key::Subtract;
        case KEY_KP4:           return za::Keyboard::Key::Numpad4;
        case KEY_KP5:           return za::Keyboard::Key::Numpad5;
        case KEY_KP6:           return za::Keyboard::Key::Numpad6;
        case KEY_KPPLUS:        return za::Keyboard::Key::Add;
        case KEY_KP1:           return za::Keyboard::Key::Numpad1;
        case KEY_KP2:           return za::Keyboard::Key::Numpad2;
        case KEY_KP3:           return za::Keyboard::Key::Numpad3;
        case KEY_KP0:           return za::Keyboard::Key::Numpad0;
        case KEY_KPDOT:         return za::Keyboard::Key::Delete;
        case KEY_RIGHTCTRL:     return za::Keyboard::Key::RControl;
        case KEY_KPSLASH:       return za::Keyboard::Key::Divide;
        case KEY_RIGHTALT:      return za::Keyboard::Key::RAlt;
        case KEY_HOME:          return za::Keyboard::Key::Home;
        case KEY_UP:            return za::Keyboard::Key::Up;
        case KEY_PAGEUP:        return za::Keyboard::Key::PageUp;
        case KEY_LEFT:          return za::Keyboard::Key::Left;
        case KEY_RIGHT:         return za::Keyboard::Key::Right;
        case KEY_END:           return za::Keyboard::Key::End;
        case KEY_DOWN:          return za::Keyboard::Key::Down;
        case KEY_PAGEDOWN:      return za::Keyboard::Key::PageDown;
        case KEY_INSERT:        return za::Keyboard::Key::Insert;
        case KEY_DELETE:        return za::Keyboard::Key::Delete;
        case KEY_PAUSE:         return za::Keyboard::Key::Pause;
        case KEY_LEFTMETA:      return za::Keyboard::Key::LSystem;
        case KEY_RIGHTMETA:     return za::Keyboard::Key::RSystem;

        case KEY_RESERVED:
        case KEY_SYSRQ:
        case KEY_CAPSLOCK:
        case KEY_NUMLOCK:
        case KEY_SCROLLLOCK:
        default:
            return za::Keyboard::Key::Unknown;
            // clang-format on
    }
}

void pushEvent(const za::Event& event)
{
    if (eventQueue.size() >= maxQueue)
        eventQueue.pop();

    eventQueue.push(event);
}

TouchSlot& atSlot(int idx)
{
    if (idx >= static_cast<int>(touchSlots.size()))
        touchSlots.resize(static_cast<base::SizeT>(idx + 1));
    return touchSlots.at(static_cast<base::SizeT>(idx));
}

void processSlots()
{
    for (auto& slot : touchSlots)
    {
        if (slot.oldId == slot.id)
        {
            pushEvent(za::Event::TouchMoved{*slot.id, slot.pos});
        }
        else
        {
            if (slot.oldId.hasValue())
                pushEvent(za::Event::TouchEnded{*slot.oldId, slot.pos});
            if (slot.id.hasValue())
                pushEvent(za::Event::TouchBegan{*slot.id, slot.pos});

            slot.oldId = slot.id;
        }
    }
}

zb::Optional<za::Event> eventProcess()
{
    const std::lock_guard lock(inputMutex);

    // Ensure that we are initialized
    initFileDescriptors();

    // This is for handling the Backspace and DEL text events, which we
    // generate based on keystrokes (and not stdin)
    static unsigned int doDeferredText = 0;
    if (doDeferredText)
    {
        const auto event = za::Event::TextEntered{doDeferredText};

        doDeferredText = 0;
        return event;
    }

    ssize_t bytesRead = 0;

    // Check all the open file descriptors for the next event
    for (auto& fileDescriptor : fileDescriptors)
    {
        input_event inputEvent{};
        bytesRead = read(fileDescriptor, &inputEvent, sizeof(inputEvent));

        while (bytesRead > 0)
        {
            if (inputEvent.type == EV_KEY)
            {
                if (const zb::Optional<za::Mouse::Button> mb = toMouseButton(inputEvent.code))
                {
                    mouseMap[*mb] = inputEvent.value;

                    if (inputEvent.value)
                        return za::Event::MouseButtonPressed{*mb, mousePos};

                    return za::Event::MouseButtonReleased{*mb, mousePos};
                }

                const za::Keyboard::Key kb = toKey(inputEvent.code);

                unsigned int special = 0;
                if ((kb == za::Keyboard::Key::Delete) || (kb == za::Keyboard::Key::Backspace))
                    special = (kb == za::Keyboard::Key::Delete) ? 127 : 8;

                if (inputEvent.value == 2)
                {
                    // key repeat events
                    //
                    if (special)
                    {
                        return za::Event::TextEntered{special};
                    }
                }
                else if (kb != za::Keyboard::Key::Unknown)
                {
                    // key down and key up events
                    //
                    keyMap[kb] = inputEvent.value;

                    if (special && inputEvent.value)
                        doDeferredText = special;

                    const auto makeKeyEvent = [&](auto keyEvent)
                    {
                        keyEvent.code     = kb;
                        keyEvent.scancode = za::Keyboard::Scan::Unknown; // TODO P2: not implemented
                        keyEvent.alt      = altDown();
                        keyEvent.control  = controlDown();
                        keyEvent.shift    = shiftDown();
                        keyEvent.system   = systemDown();
                        return keyEvent;
                    };

                    if (inputEvent.value)
                        return makeKeyEvent(za::Event::KeyPressed{});

                    return makeKeyEvent(za::Event::KeyReleased{});
                }
            }
            else if (inputEvent.type == EV_REL)
            {
                bool posChange = false;
                switch (inputEvent.code)
                {
                    case REL_X:
                        mousePos.x += inputEvent.value;
                        posChange = true;
                        break;

                    case REL_Y:
                        mousePos.y += inputEvent.value;
                        posChange = true;
                        break;

                    case REL_WHEEL:
                        za::Event::MouseWheelScrolled mouseWheelScrolled;
                        mouseWheelScrolled.delta    = static_cast<float>(inputEvent.value);
                        mouseWheelScrolled.position = mousePos;
                        return mouseWheelScrolled;
                }
                if (posChange)
                {
                    return za::Event::MouseMoved{mousePos};
                }
            }
            else if (inputEvent.type == EV_ABS)
            {
                switch (inputEvent.code)
                {
                    case ABS_MT_SLOT:
                        currentSlot = inputEvent.value;
                        touchFd     = fileDescriptor;
                        break;
                    case ABS_MT_TRACKING_ID:
                        atSlot(currentSlot).id = inputEvent.value >= 0 ? base::Optional(inputEvent.value) : base::nullOpt;
                        touchFd = fileDescriptor;
                        break;
                    case ABS_MT_POSITION_X:
                        atSlot(currentSlot).pos.x = inputEvent.value;
                        touchFd                   = fileDescriptor;
                        break;
                    case ABS_MT_POSITION_Y:
                        atSlot(currentSlot).pos.y = inputEvent.value;
                        touchFd                   = fileDescriptor;
                        break;
                }
            }
            else if (inputEvent.type == EV_SYN && inputEvent.code == SYN_REPORT && fileDescriptor == touchFd)
            {
                // This pushes events directly to the queue, because it
                // can generate more than one event.
                processSlots();
            }

            bytesRead = read(fileDescriptor, &inputEvent, sizeof(inputEvent));
        }

        if ((bytesRead < 0) && (errno != EAGAIN))
            za::priv::err() << " Error: " << std::strerror(errno);
    }
    // Finally check if there is a Text event on stdin
    //
    // We only clear the ICANON flag for the time of reading

    newTerminalConfig.c_lflag &= ~static_cast<tcflag_t>(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalConfig);

    timeval timeout{};
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    unsigned char code = 0;

    fd_set readFDSet;
    FD_ZERO(&readFDSet);
    FD_SET(STDIN_FILENO, &readFDSet);
    int ready = select(STDIN_FILENO + 1, &readFDSet, nullptr, nullptr, &timeout);

    if (ready > 0 && FD_ISSET(STDIN_FILENO, &readFDSet))
        bytesRead = read(STDIN_FILENO, &code, 1);

    if ((code == 127) || (code == 8)) // Suppress 127 (DEL) to 8 (BACKSPACE)
        code = 0;
    else if (code == 27) // ESC
    {
        // Suppress ANSI escape sequences
        FD_ZERO(&readFDSet);
        FD_SET(STDIN_FILENO, &readFDSet);
        ready = select(STDIN_FILENO + 1, &readFDSet, nullptr, nullptr, &timeout);
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &readFDSet))
        {
            unsigned char tempBuffer[16];
            bytesRead = read(STDIN_FILENO, tempBuffer, 16);
            code      = 0;
        }
    }

    (void)bytesRead; // Ignore clang-tidy dead store warning

    newTerminalConfig.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalConfig);

    if (code > 0)
    {
        // TODO P2: Proper unicode handling
        return za::Event::TextEntered{code};
    }

    // No events available
    return base::nullOpt;
}

// assumes inputMutex is locked
void update()
{
    while (const base::Optional event = eventProcess())
        pushEvent(*event);
}
} // namespace


namespace za::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key key)
{
    const std::lock_guard lock(inputMutex);
    if ((static_cast<int>(key) < 0) || (static_cast<int>(key) >= static_cast<int>(keyMap.size())))
        return false;

    update();
    return keyMap[key];
}


////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode /* code */)
{
    // TODO P2: not implemented
    priv::err() << "za::Keyboard::isKeyPressed(Keyboard::Scancode) is not implemented for DRM.";
    return false;
}


////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode /* code */)
{
    // TODO P2: not implemented
    priv::err() << "za::Keyboard::localize is not implemented for DRM.";
    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key /* key */)
{
    // TODO P2: not implemented
    priv::err() << "za::Keyboard::delocalize is not implemented for DRM.";
    return Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode /* code */)
{
    // TODO P2: not implemented
    priv::err() << "za::Keyboard::getDescription is not implemented for DRM.";
    return "";
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool /*visible*/)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button button)
{
    const std::lock_guard lock(inputMutex);
    if ((static_cast<int>(button) < 0) || (static_cast<int>(button) >= static_cast<int>(mouseMap.size())))
        return false;

    update();
    return mouseMap[button];
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition()
{
    const std::lock_guard lock(inputMutex);
    return mousePos;
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& /*relativeTo*/)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position)
{
    const std::lock_guard lock(inputMutex);
    mousePos = position;
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position, const WindowBase& /*relativeTo*/)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int finger)
{
    return base::anyOf(touchSlots.cbegin(),
                       touchSlots.cend(),
                       [finger](const TouchSlot& slot) { return slot.id == finger; });
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger)
{
    for (const auto& slot : touchSlots)
    {
        if (slot.id == finger)
            return slot.pos;
    }

    return {};
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger, const WindowBase& /*relativeTo*/)
{
    return getTouchPosition(finger);
}


////////////////////////////////////////////////////////////
base::Optional<Event> checkEvent()
{
    const std::lock_guard lock(inputMutex);

    if (!eventQueue.empty())
    {
        auto event = base::makeOptional(eventQueue.front());
        eventQueue.pop();

        return event;
    }

    if (const base::Optional event = eventProcess())
    {
        return event;
    }

    // In the case of multitouch, eventProcess() could have returned false
    // but added events directly to the queue.  (This is ugly, but I'm not
    // sure of a good way to handle generating multiple events at once.)
    if (!eventQueue.empty())
    {
        auto event = base::makeOptional(eventQueue.front());
        eventQueue.pop();

        return event;
    }

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
void setTerminalConfig()
{
    const std::lock_guard lock(inputMutex);
    initFileDescriptors();

    tcgetattr(STDIN_FILENO, &newTerminalConfig);               // get current terminal config
    oldTerminalConfig = newTerminalConfig;                     // create a backup
    newTerminalConfig.c_lflag &= ~static_cast<tcflag_t>(ECHO); // disable console feedback
    newTerminalConfig.c_lflag &= ~static_cast<tcflag_t>(ISIG); // disable signals
    newTerminalConfig.c_lflag |= ICANON;                       // disable noncanonical mode
    newTerminalConfig.c_iflag |= IGNCR;                        // ignore carriage return
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalConfig);      // set our new config
    tcflush(STDIN_FILENO, TCIFLUSH);                           // flush the buffer
}


////////////////////////////////////////////////////////////
void restoreTerminalConfig()
{
    const std::lock_guard lock(inputMutex);
    initFileDescriptors();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalConfig); // restore terminal config
    tcflush(STDIN_FILENO, TCIFLUSH);                      // flush the buffer
}

} // namespace za::priv::InputImpl
