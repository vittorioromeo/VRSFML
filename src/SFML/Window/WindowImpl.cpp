#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/JoystickCapabilities.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/JoystickState.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/Sensor.hpp"
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Utf.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/EnumArray.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <queue>
#include <string>


////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)
    #include "SFML/Window/VulkanImpl.hpp"
    #include "SFML/Window/Win32/Utils.hpp"
#elif defined(SFML_SYSTEM_LINUX_OR_BSD)
    #if defined(SFML_USE_DRM)
        #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
    #else
        #include "SFML/Window/VulkanImpl.hpp"
    #endif
#elif defined(SFML_SYSTEM_MACOS)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
#elif defined(SFML_SYSTEM_IOS)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
#elif defined(SFML_SYSTEM_ANDROID)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
#elif defined(SFML_SYSTEM_EMSCRIPTEN)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
#endif


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
// Yes, this is a rather weird namespace.
namespace WindowImplImpl
{
////////////////////////////////////////////////////////////
struct TouchInfo
{
    unsigned int     normalizedIndex;
    sf::Vector2i     position;
    sf::WindowHandle handle;
};


////////////////////////////////////////////////////////////
bool touchIndexPool[32]{}; // Keeps track of which finger indices are in use


////////////////////////////////////////////////////////////
[[nodiscard]] int findFirstNormalizedTouchIndex()
{
    for (sf::base::SizeT i = 0u; i < 32u; ++i)
        if (!touchIndexPool[i])
            return static_cast<int>(i);

    sf::priv::err() << "No available touch index\n";
    return -1;
}


////////////////////////////////////////////////////////////
ankerl::unordered_dense::map<SDL_FingerID, TouchInfo> touchMap;


////////////////////////////////////////////////////////////
const sf::priv::WindowImpl* fullscreenWindow = nullptr;


////////////////////////////////////////////////////////////
ankerl::unordered_dense::map<SDL_WindowID, sf::priv::WindowImpl*> windowImplMap;

} // namespace WindowImplImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct WindowImpl::Impl
{
    std::queue<Event> events; //!< Queue of available events

    JoystickState joystickStates[Joystick::MaxCount]{};    //!< Previous state of the joysticks
    bool          joystickConnected[Joystick::MaxCount]{}; //!< Previous connection state of the joysticks

    base::EnumArray<Sensor::Type, Vector3f, Sensor::Count> sensorValue; //!< Previous value of the sensors

    float joystickThreshold{0.1f}; //!< Joystick threshold (minimum motion for "move" event to be generated)

    base::EnumArray<Joystick::Axis, float, Joystick::AxisCount>
        previousAxes[Joystick::MaxCount]{}; //!< Position of each axis last time a move event triggered, in range [-100, 100]

    base::Optional<Vector2u> minimumSize; //!< Minimum window size
    base::Optional<Vector2u> maximumSize; //!< Maximum window size

    SDL_Window* sdlWindow; //!< SDL window handle

    bool keyRepeatEnabled = false; //!< Is the key repeat feature enabled?

    bool isExternal = false; //!< Is the window created externally?

    explicit Impl(const char* context, SDL_Window* theSDLWindow, const bool theIsExternal) :
    sdlWindow{theSDLWindow},
    isExternal{theIsExternal}
    {
        if (!sdlWindow)
        {
            err() << "Failed to create window created from " << context << ": " << SDL_GetError();
            return;
        }

        if (!SDL_StartTextInput(sdlWindow)) // TODO P1: might not want to call this on mobiles
            err() << "Failed to start text input for window created from " << context << ": " << SDL_GetError();
    }

    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&& rhs)            = delete;
    Impl& operator=(Impl&& rhs) = delete;

    ~Impl()
    {
        if (!SDL_StopTextInput(sdlWindow))
            err() << "Failed to stop text input for window: " << SDL_GetError();

        if (!isExternal)
            SDL_DestroyWindow(sdlWindow);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDL_WindowID getWindowID() const
    {
        const auto result = SDL_GetWindowID(sdlWindow);

        if (result == 0)
            err() << "Failed to get window ID: " << SDL_GetError();

        return result;
    }
};


////////////////////////////////////////////////////////////
void WindowImpl::processSDLEvent(const SDL_Event& e)
{
    const auto et = static_cast<SDL_EventType>(e.type);

    switch (et)
    {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            pushEvent(Event::Closed{});
            break;
        }

        case SDL_EVENT_WINDOW_RESIZED:
        {
            pushEvent(Event::Resized{Vector2i{e.window.data1, e.window.data2}.toVector2u()});
            break;
        }

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        {
            pushEvent(Event::MouseEntered{});
            break;
        }

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        {
            pushEvent(Event::MouseLeft{});
            break;
        }

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            pushEvent(Event::FocusGained{});
            break;
        }

        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            pushEvent(Event::FocusLost{});
            break;
        }

        case SDL_EVENT_KEY_DOWN:
        {
            if (!m_impl->keyRepeatEnabled && e.key.repeat)
                return;

            pushEvent(Event::KeyPressed{.code     = mapSDLKeycodeToSFML(e.key.key),
                                        .scancode = mapSDLScancodeToSFML(e.key.scancode),
                                        .alt      = static_cast<bool>(e.key.mod & SDL_KMOD_ALT),
                                        .control  = static_cast<bool>(e.key.mod & SDL_KMOD_CTRL),
                                        .shift    = static_cast<bool>(e.key.mod & SDL_KMOD_SHIFT),
                                        .system   = static_cast<bool>(e.key.mod & SDL_KMOD_GUI)});
            break;
        }

        case SDL_EVENT_KEY_UP:
        {
            pushEvent(Event::KeyReleased{.code     = mapSDLKeycodeToSFML(e.key.key),
                                         .scancode = mapSDLScancodeToSFML(e.key.scancode),
                                         .alt      = static_cast<bool>(e.key.mod & SDL_KMOD_ALT),
                                         .control  = static_cast<bool>(e.key.mod & SDL_KMOD_CTRL),
                                         .shift    = static_cast<bool>(e.key.mod & SDL_KMOD_SHIFT),
                                         .system   = static_cast<bool>(e.key.mod & SDL_KMOD_GUI)});
            break;
        }

        case SDL_EVENT_TEXT_INPUT:
        {
            char32_t     unicode   = 0;
            const char*  keyBuffer = e.text.text;
            const size_t length    = SFML_BASE_STRLEN(keyBuffer);
            const auto*  iter      = keyBuffer;

            while (iter < keyBuffer + length)
            {
                iter = Utf8::decode(iter, keyBuffer + length, unicode, 0);
                if (unicode != 0)
                    pushEvent(Event::TextEntered{unicode});
            }

            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
            pushEvent(Event::MouseMoved{
                .position = {static_cast<int>(e.motion.x), static_cast<int>(e.motion.y)},
            });
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            pushEvent(Event::MouseButtonPressed{
                .button   = getButtonFromSDLButton(e.button.button),
                .position = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)},
            });
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            pushEvent(Event::MouseButtonReleased{
                .button   = getButtonFromSDLButton(e.button.button),
                .position = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)},
            });
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL:
        {
            pushEvent(Event::MouseWheelScrolled{
                .wheel    = Mouse::Wheel::Vertical, // TODO P0: horizontal wheel support?
                .delta    = static_cast<float>(e.wheel.y),
                .position = {static_cast<int>(e.wheel.x), static_cast<int>(e.wheel.y)},
            });
            break;
        }

        case SDL_EVENT_FINGER_DOWN:
        {
            const SDL_TouchFingerEvent& fingerEvent = e.tfinger; // TODO P0: add touch device?
            const auto touchPos = Vector2f{fingerEvent.x, fingerEvent.y}.componentWiseMul(getSize().toVector2f()).toVector2i();

            SFML_BASE_ASSERT(!WindowImplImpl::touchMap.contains(fingerEvent.fingerID));

            const int normalizedIndex = WindowImplImpl::findFirstNormalizedTouchIndex();
            if (normalizedIndex == -1)
                break;

            const auto fingerIdx                      = static_cast<unsigned int>(normalizedIndex);
            WindowImplImpl::touchIndexPool[fingerIdx] = true;
            WindowImplImpl::touchMap.emplace(fingerEvent.fingerID,
                                             WindowImplImpl::TouchInfo{fingerIdx, touchPos, getNativeHandle()});

            pushEvent(sf::Event::TouchBegan{fingerIdx, touchPos, fingerEvent.pressure});
            break;
        }

        case SDL_EVENT_FINGER_UP:
        {
            const SDL_TouchFingerEvent& fingerEvent = e.tfinger;
            const auto touchPos = Vector2f{fingerEvent.x, fingerEvent.y}.componentWiseMul(getSize().toVector2f()).toVector2i();

            SFML_BASE_ASSERT(WindowImplImpl::touchMap.contains(fingerEvent.fingerID));
            const auto [fingerIdx, pos, handle] = WindowImplImpl::touchMap[fingerEvent.fingerID];

            WindowImplImpl::touchIndexPool[fingerIdx] = false;
            WindowImplImpl::touchMap.erase(fingerEvent.fingerID);

            pushEvent(sf::Event::TouchEnded{fingerIdx, touchPos, fingerEvent.pressure});
            break;
        }

        case SDL_EVENT_FINGER_MOTION:
        {
            const SDL_TouchFingerEvent& fingerEvent = e.tfinger;
            const auto touchPos = Vector2f{fingerEvent.x, fingerEvent.y}.componentWiseMul(getSize().toVector2f()).toVector2i();

            SFML_BASE_ASSERT(WindowImplImpl::touchMap.contains(fingerEvent.fingerID));
            const auto [fingerIdx, pos, handle] = WindowImplImpl::touchMap[fingerEvent.fingerID];

            pushEvent(sf::Event::TouchMoved{fingerIdx, touchPos, fingerEvent.pressure});
            break;
        }

        case SDL_EVENT_FINGER_CANCELED: // TODO
        {
            break;
        }

            // unused
        case SDL_EVENT_QUIT:
        case SDL_EVENT_FIRST:
        case SDL_EVENT_TERMINATING:
        case SDL_EVENT_LOW_MEMORY:
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
        case SDL_EVENT_DID_ENTER_BACKGROUND:
        case SDL_EVENT_WILL_ENTER_FOREGROUND:
        case SDL_EVENT_DID_ENTER_FOREGROUND:
        case SDL_EVENT_LOCALE_CHANGED:
        case SDL_EVENT_SYSTEM_THEME_CHANGED:
        case SDL_EVENT_DISPLAY_ORIENTATION:
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
        case SDL_EVENT_DISPLAY_MOVED:
        case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
        case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
        case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_HIT_TEST:
        case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
        case SDL_EVENT_WINDOW_OCCLUDED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        case SDL_EVENT_WINDOW_DESTROYED:
        case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
        case SDL_EVENT_TEXT_EDITING:
        case SDL_EVENT_KEYMAP_CHANGED:
        case SDL_EVENT_KEYBOARD_ADDED:
        case SDL_EVENT_KEYBOARD_REMOVED:
        case SDL_EVENT_TEXT_EDITING_CANDIDATES:
        case SDL_EVENT_MOUSE_ADDED:
        case SDL_EVENT_MOUSE_REMOVED:
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        case SDL_EVENT_JOYSTICK_BALL_MOTION:
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        case SDL_EVENT_JOYSTICK_ADDED:
        case SDL_EVENT_JOYSTICK_REMOVED:
        case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
        case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
        case SDL_EVENT_GAMEPAD_REMAPPED:
        case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
        case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
        case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
        case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
        case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
        case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
        case SDL_EVENT_CLIPBOARD_UPDATE:
        case SDL_EVENT_DROP_FILE:
        case SDL_EVENT_DROP_TEXT:
        case SDL_EVENT_DROP_BEGIN:
        case SDL_EVENT_DROP_COMPLETE:
        case SDL_EVENT_DROP_POSITION:
        case SDL_EVENT_AUDIO_DEVICE_ADDED:
        case SDL_EVENT_AUDIO_DEVICE_REMOVED:
        case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
        case SDL_EVENT_SENSOR_UPDATE:
        case SDL_EVENT_PEN_PROXIMITY_IN:
        case SDL_EVENT_PEN_PROXIMITY_OUT:
        case SDL_EVENT_PEN_DOWN:
        case SDL_EVENT_PEN_UP:
        case SDL_EVENT_PEN_BUTTON_DOWN:
        case SDL_EVENT_PEN_BUTTON_UP:
        case SDL_EVENT_PEN_MOTION:
        case SDL_EVENT_PEN_AXIS:
        case SDL_EVENT_CAMERA_DEVICE_ADDED:
        case SDL_EVENT_CAMERA_DEVICE_REMOVED:
        case SDL_EVENT_CAMERA_DEVICE_APPROVED:
        case SDL_EVENT_CAMERA_DEVICE_DENIED:
        case SDL_EVENT_RENDER_TARGETS_RESET:
        case SDL_EVENT_RENDER_DEVICE_RESET:
        case SDL_EVENT_RENDER_DEVICE_LOST:
        case SDL_EVENT_PRIVATE0:
        case SDL_EVENT_PRIVATE1:
        case SDL_EVENT_PRIVATE2:
        case SDL_EVENT_PRIVATE3:
        case SDL_EVENT_POLL_SENTINEL:
        case SDL_EVENT_USER:
        case SDL_EVENT_LAST:
        case SDL_EVENT_ENUM_PADDING:
            break;
    }
}


////////////////////////////////////////////////////////////
base::UniquePtr<WindowImpl> WindowImpl::create(WindowSettings windowSettings)
{
    // Fullscreen style requires some tests
    if (windowSettings.fullscreen)
    {
        // Make sure there's not already a fullscreen window (only one is allowed)
        if (WindowImplImpl::fullscreenWindow != nullptr)
        {
            err() << "Creating two fullscreen windows is not allowed, switching to windowed mode";
            windowSettings.fullscreen = false;
        }
        else
        {
            VideoMode videoMode{windowSettings.size, windowSettings.bitsPerPixel};

            // Make sure that the chosen video mode is compatible
            if (!videoMode.isValid())
            {
                err() << "The requested video mode is not available, switching to a valid mode";

                SFML_BASE_ASSERT(!VideoModeUtils::getFullscreenModes().empty() && "No video modes available");
                videoMode = VideoModeUtils::getFullscreenModes()[0];

                err() << "  VideoMode: { size: { " << videoMode.size.x << ", " << videoMode.size.y
                      << " }, bitsPerPixel: " << videoMode.bitsPerPixel << " }";
            }
        }
    }

// Check validity of style according to the underlying platform
#if defined(SFML_SYSTEM_IOS) || defined(SFML_SYSTEM_ANDROID)
    if (windowSettings.fullscreen)
        windowSettings.hasTitlebar = false;
    else
        windowSettings.hasTitlebar = true;
#else
    if (windowSettings.closable || windowSettings.resizable)
        windowSettings.hasTitlebar = true;
#endif

    SDL_Window* sdlWindowPtr = SDL_CreateWindow(windowSettings.title.toAnsiString<std::string>().data(),
                                                static_cast<int>(windowSettings.size.x),
                                                static_cast<int>(windowSettings.size.y),
                                                makeSDLWindowFlagsFromWindowSettings(windowSettings));

    auto* windowImplPtr = new WindowImpl{"window settings",
                                         static_cast<void*>(sdlWindowPtr),
                                         /* isExternal */ false};

#ifdef SFML_SYSTEM_WINDOWS
    {
        // See https://github.com/libsdl-org/SDL/issues/12791

        const auto desktopModeSize = sf::VideoModeUtils::getDesktopMode().size;
        if (!windowSettings.hasTitlebar && !windowSettings.fullscreen && windowSettings.size == desktopModeSize)
        {
            void* hwnd = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindowPtr),
                                                SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                                                nullptr);

            priv::setWindowBorderless(hwnd, windowSettings.size.x, windowSettings.size.y);
        }
    }
#endif

    if (windowSettings.fullscreen)
        WindowImplImpl::fullscreenWindow = windowImplPtr;

    return base::UniquePtr<WindowImpl>{windowImplPtr};
}


////////////////////////////////////////////////////////////
base::UniquePtr<WindowImpl> WindowImpl::create(WindowHandle handle)
{
    auto* windowImplPtr = new WindowImpl{"handle",
                                         static_cast<void*>(
                                             SDL_CreateWindowWithProperties(makeSDLWindowPropertiesFromHandle(handle))),
                                         /* isExternal */ true};

    return base::UniquePtr<WindowImpl>{windowImplPtr};
}


////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl()
{
    if (WindowImplImpl::fullscreenWindow == this)
        WindowImplImpl::fullscreenWindow = nullptr;

    // Unregister the window from the global map
    const auto windowId = m_impl->getWindowID();
    SFML_BASE_ASSERT(WindowImplImpl::windowImplMap.contains(windowId));
    WindowImplImpl::windowImplMap.erase(windowId);
}


////////////////////////////////////////////////////////////
base::Optional<Vector2u> WindowImpl::getMinimumSize() const
{
    return m_impl->minimumSize;
}


////////////////////////////////////////////////////////////
base::Optional<Vector2u> WindowImpl::getMaximumSize() const
{
    return m_impl->maximumSize;
}


////////////////////////////////////////////////////////////
void WindowImpl::setJoystickThreshold(float threshold)
{
    m_impl->joystickThreshold = threshold;
}


////////////////////////////////////////////////////////////
void WindowImpl::setMinimumSize(const base::Optional<Vector2u>& minimumSize)
{
    m_impl->minimumSize = minimumSize;
}


////////////////////////////////////////////////////////////
void WindowImpl::setMaximumSize(const base::Optional<Vector2u>& maximumSize)
{
    m_impl->maximumSize = maximumSize;
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::waitEvent(Time timeout)
{
    sf::Clock clock;

    const auto timedOut = [&clock, timeout, startTime = clock.getElapsedTime()]
    {
        const bool infiniteTimeout = timeout == Time{};
        return !infiniteTimeout && (clock.getElapsedTime() - startTime) >= timeout;
    };

    // If the event queue is empty, let's first check if new events are available from the OS
    if (m_impl->events.empty())
        populateEventQueue();

    // Here we use a manual wait loop instead of the optimized wait-event provided by the OS,
    // so that we don't skip joystick events (which require polling)
    while (m_impl->events.empty() && !timedOut())
    {
        sleep(milliseconds(10));
        populateEventQueue();
    }

    return popEvent();
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::pollEvent()
{
    // If the event queue is empty, let's first check if new events are available from the OS
    if (m_impl->events.empty())
        populateEventQueue();

    return popEvent();
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::popEvent()
{
    base::Optional<Event> event; // Use a single local variable for NRVO

    if (!m_impl->events.empty())
    {
        event.emplace(m_impl->events.front());
        m_impl->events.pop();
    }

    return event;
}


////////////////////////////////////////////////////////////
WindowImpl::WindowImpl(const char* context, void* sdlWindow, const bool isExternal) :
m_impl{context, static_cast<SDL_Window*>(sdlWindow), isExternal}
{
    auto& joystickManager = WindowContext::getJoystickManager();

    // Get the initial joystick states
    joystickManager.update();

    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
    {
        m_impl->joystickStates[i]    = joystickManager.getState(i);
        m_impl->joystickConnected[i] = joystickManager.isConnected(i);
        m_impl->previousAxes[i].fill(0.f);
    }

    // Get the initial sensor states
    for (Vector3f& vec : m_impl->sensorValue.data)
        vec = Vector3f{0.f, 0.f, 0.f};

    // Register the window in the global map
    const auto windowId = m_impl->getWindowID();
    SFML_BASE_ASSERT(!WindowImplImpl::windowImplMap.contains(windowId));
    WindowImplImpl::windowImplMap.emplace(windowId, this); // Needs address stability
}


////////////////////////////////////////////////////////////
void WindowImpl::pushEvent(const Event& event)
{
    m_impl->events.push(event);
}


////////////////////////////////////////////////////////////
void WindowImpl::processJoystickEvents()
{
    auto& joystickManager = WindowContext::getJoystickManager();

    // First update the global joystick states
    joystickManager.update();

    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
    {
        // Copy the previous state of the joystick and get the new one
        const JoystickState previousState = m_impl->joystickStates[i];
        m_impl->joystickStates[i]         = joystickManager.getState(i);

        // Copy the previous connection state of the joystick and get the new one
        const bool previousConnected = m_impl->joystickConnected[i];
        m_impl->joystickConnected[i] = joystickManager.isConnected(i);

        const bool connected = m_impl->joystickConnected[i];
        if (previousConnected != connected)
        {
            if (connected)
                pushEvent(Event::JoystickConnected{i});
            else
                pushEvent(Event::JoystickDisconnected{i});

            // Clear previous axes positions
            if (connected)
                m_impl->previousAxes[i].fill(0.f);
        }

        if (!connected)
            continue;

        const JoystickCapabilities caps = joystickManager.getCapabilities(i);

        // Axes
        for (unsigned int j = 0; j < Joystick::AxisCount; ++j)
        {
            const auto axis = static_cast<Joystick::Axis>(j);
            if (!caps.axes[axis])
                continue;

            const float prevPos = m_impl->previousAxes[i][axis];
            const float currPos = m_impl->joystickStates[i].axes[axis];
            if (base::fabs(currPos - prevPos) >= m_impl->joystickThreshold)
            {
                pushEvent(Event::JoystickMoved{i, axis, currPos});
                m_impl->previousAxes[i][axis] = currPos;
            }
        }

        // Buttons
        for (unsigned int j = 0; j < caps.buttonCount; ++j)
        {
            const bool prevPressed = previousState.buttons[j];
            const bool currPressed = m_impl->joystickStates[i].buttons[j];

            if (prevPressed != currPressed)
            {
                if (currPressed)
                    pushEvent(Event::JoystickButtonPressed{i, j});
                else
                    pushEvent(Event::JoystickButtonReleased{i, j});
            }
        }
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::processSensorEvents()
{
    // First update the sensor states
    auto& sensorManager = WindowContext::getSensorManager();
    sensorManager.update();

    for (unsigned int i = 0; i < Sensor::Count; ++i)
    {
        const auto sensor = static_cast<Sensor::Type>(i);

        // Only process enabled sensors
        if (!sensorManager.isEnabled(sensor))
            continue;

        // Copy the previous value of the sensor and get the new one
        const Vector3f previousValue = m_impl->sensorValue[sensor];
        m_impl->sensorValue[sensor]  = sensorManager.getValue(sensor);

        // If the value has changed, trigger an event
        if (m_impl->sensorValue[sensor] != previousValue) // TODO P2: use a threshold?
            pushEvent(Event::SensorChanged{sensor, m_impl->sensorValue[sensor]});
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::populateEventQueue()
{
    processJoystickEvents();
    processSensorEvents();
    WindowImpl::processEvents();

#ifdef SFML_SYSTEM_EMSCRIPTEN
    SDL_SyncWindow(m_impl->sdlWindow);
#endif
}


////////////////////////////////////////////////////////////
bool WindowImpl::createVulkanSurface([[maybe_unused]] const Vulkan::VulkanSurfaceData& vulkanSurfaceData) const
{
#ifdef SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    return false;

#else

    return VulkanImpl::createVulkanSurface(vulkanSurfaceData.instance,
                                           getNativeHandle(),
                                           vulkanSurfaceData.surface,
                                           vulkanSurfaceData.allocator);

#endif
}


////////////////////////////////////////////////////////////
Vector2i WindowImpl::getPosition() const
{
    Vector2i result;

    if (!SDL_GetWindowPosition(m_impl->sdlWindow, &result.x, &result.y))
        err() << "Failed to get window position: " << SDL_GetError();

    return result;
}


////////////////////////////////////////////////////////////
void WindowImpl::setPosition(Vector2i position)
{
    SDL_SetWindowPosition(m_impl->sdlWindow, position.x, position.y);
}


////////////////////////////////////////////////////////////
Vector2u WindowImpl::getSize() const
{
    Vector2i result;

    if (!SDL_GetWindowSize(m_impl->sdlWindow, &result.x, &result.y))
        err() << "Failed to get window size: " << SDL_GetError();

    return result.toVector2u();
}


////////////////////////////////////////////////////////////
void WindowImpl::setSize(Vector2u size)
{
    if (!SDL_SetWindowSize(m_impl->sdlWindow, static_cast<int>(size.x), static_cast<int>(size.y)))
        err() << "Failed to set window size: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImpl::setTitle(const String& title)
{
    if (!SDL_SetWindowTitle(m_impl->sdlWindow, title.toAnsiString<std::string>().data()))
        err() << "Failed to set window title: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImpl::setIcon(Vector2u size, const base::U8* pixels)
{
    auto surface = getSDLLayerSingleton().createSurfaceFromPixels(size, pixels);
    if (surface == nullptr)
    {
        err() << "Failed to set icon";
        return;
    }

    if (!SDL_SetWindowIcon(m_impl->sdlWindow, surface.get()))
        err() << "Failed to set window icon: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImpl::setVisible(const bool visible)
{
    if (visible)
    {
        if (!SDL_ShowWindow(m_impl->sdlWindow))
            err() << "Failed to show window: " << SDL_GetError();
    }
    else
    {
        if (!SDL_HideWindow(m_impl->sdlWindow))
            err() << "Failed to hide window: " << SDL_GetError();
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::setMouseCursorVisible(const bool visible)
{
    if (visible)
    {
        if (!SDL_ShowCursor())
            err() << "Failed to show cursor: " << SDL_GetError();
    }
    else
    {
        if (!SDL_HideCursor())
            err() << "Failed to hide cursor: " << SDL_GetError();
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::setMouseCursorGrabbed(const bool grabbed)
{
    if (!SDL_SetWindowMouseGrab(m_impl->sdlWindow, grabbed))
        err() << "Failed to set window mouse grab: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImpl::setMouseCursor(void* cursor)
{
    SDL_SetCursor(static_cast<SDL_Cursor*>(cursor));
}


////////////////////////////////////////////////////////////
void WindowImpl::setKeyRepeatEnabled(bool enabled)
{
    m_impl->keyRepeatEnabled = enabled;
}


////////////////////////////////////////////////////////////
void WindowImpl::requestFocus()
{
    if (!SDL_RaiseWindow(m_impl->sdlWindow))
        err() << "Failed to raise window: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
bool WindowImpl::hasFocus() const
{
    return SDL_GetWindowFlags(m_impl->sdlWindow) & SDL_WINDOW_INPUT_FOCUS;
}


////////////////////////////////////////////////////////////
float WindowImpl::getWindowDisplayScale() const
{
    return priv::getSDLLayerSingleton().getWindowDisplayScale(*m_impl->sdlWindow);
}


////////////////////////////////////////////////////////////
WindowHandle WindowImpl::getNativeHandle() const
{
    const auto props = SDL_GetWindowProperties(m_impl->sdlWindow);

    return static_cast<WindowHandle>(
#if defined(SFML_SYSTEM_WINDOWS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)
#elif defined(SFML_SYSTEM_LINUX_OR_BSD)
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)
#elif defined(SFML_SYSTEM_MACOS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_IOS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_ANDROID)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_EMSCRIPTEN)
        SDL_GetStringProperty(props, SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING, nullptr)
#endif
    );
}


////////////////////////////////////////////////////////////
SDL_Window* WindowImpl::getSDLHandle() const
{
    return m_impl->sdlWindow;
}


////////////////////////////////////////////////////////////
void WindowImpl::processEvents()
{
    const auto dispatchSDLEvent = [&](const SDL_WindowID windowID, const SDL_Event& e)
    {
        const auto it = WindowImplImpl::windowImplMap.find(windowID);
        if (it == WindowImplImpl::windowImplMap.end())
            return;

        it->second->processSDLEvent(e);
    };

    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        const auto et = static_cast<SDL_EventType>(e.type);

        switch (et)
        {
            case SDL_EVENT_FIRST:
            case SDL_EVENT_QUIT:
            case SDL_EVENT_TERMINATING:
            case SDL_EVENT_LOW_MEMORY:
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
            case SDL_EVENT_DID_ENTER_BACKGROUND:
            case SDL_EVENT_WILL_ENTER_FOREGROUND:
            case SDL_EVENT_DID_ENTER_FOREGROUND:
            case SDL_EVENT_LOCALE_CHANGED:
            case SDL_EVENT_SYSTEM_THEME_CHANGED:
            case SDL_EVENT_DISPLAY_ORIENTATION:
            case SDL_EVENT_DISPLAY_ADDED:
            case SDL_EVENT_DISPLAY_REMOVED:
            case SDL_EVENT_DISPLAY_MOVED:
            case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
            case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
            case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
                break;

            case SDL_EVENT_WINDOW_SHOWN:
            case SDL_EVENT_WINDOW_HIDDEN:
            case SDL_EVENT_WINDOW_EXPOSED:
            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_WINDOW_HIT_TEST:
            case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            case SDL_EVENT_WINDOW_OCCLUDED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_DESTROYED:
            case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
                dispatchSDLEvent(e.window.windowID, e);
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                dispatchSDLEvent(e.key.windowID, e);
                break;

            case SDL_EVENT_TEXT_EDITING:
            case SDL_EVENT_TEXT_INPUT:
                dispatchSDLEvent(e.text.windowID, e);
                break;

            case SDL_EVENT_KEYMAP_CHANGED:
            case SDL_EVENT_KEYBOARD_ADDED:
            case SDL_EVENT_KEYBOARD_REMOVED:
            case SDL_EVENT_TEXT_EDITING_CANDIDATES:
                break;

            case SDL_EVENT_MOUSE_MOTION:
                dispatchSDLEvent(e.motion.windowID, e);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                dispatchSDLEvent(e.button.windowID, e);
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                dispatchSDLEvent(e.wheel.windowID, e);
                break;

            case SDL_EVENT_MOUSE_ADDED:
            case SDL_EVENT_MOUSE_REMOVED:
            case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            case SDL_EVENT_JOYSTICK_BALL_MOTION:
            case SDL_EVENT_JOYSTICK_HAT_MOTION:
            case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            case SDL_EVENT_JOYSTICK_BUTTON_UP:
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_JOYSTICK_REMOVED:
            case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
            case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
            case SDL_EVENT_GAMEPAD_REMAPPED:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
            case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
            case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
            case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
                break;

            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_FINGER_CANCELED:
                dispatchSDLEvent(e.tfinger.windowID, e);
                break;

            case SDL_EVENT_CLIPBOARD_UPDATE:
            case SDL_EVENT_DROP_FILE:
            case SDL_EVENT_DROP_TEXT:
            case SDL_EVENT_DROP_BEGIN:
            case SDL_EVENT_DROP_COMPLETE:
            case SDL_EVENT_DROP_POSITION:
            case SDL_EVENT_AUDIO_DEVICE_ADDED:
            case SDL_EVENT_AUDIO_DEVICE_REMOVED:
            case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
            case SDL_EVENT_SENSOR_UPDATE:
            case SDL_EVENT_PEN_PROXIMITY_IN:
            case SDL_EVENT_PEN_PROXIMITY_OUT:
            case SDL_EVENT_PEN_DOWN:
            case SDL_EVENT_PEN_UP:
            case SDL_EVENT_PEN_BUTTON_DOWN:
            case SDL_EVENT_PEN_BUTTON_UP:
            case SDL_EVENT_PEN_MOTION:
            case SDL_EVENT_PEN_AXIS:
            case SDL_EVENT_CAMERA_DEVICE_ADDED:
            case SDL_EVENT_CAMERA_DEVICE_REMOVED:
            case SDL_EVENT_CAMERA_DEVICE_APPROVED:
            case SDL_EVENT_CAMERA_DEVICE_DENIED:
            case SDL_EVENT_RENDER_TARGETS_RESET:
            case SDL_EVENT_RENDER_DEVICE_RESET:
            case SDL_EVENT_RENDER_DEVICE_LOST:
            case SDL_EVENT_PRIVATE0:
            case SDL_EVENT_PRIVATE1:
            case SDL_EVENT_PRIVATE2:
            case SDL_EVENT_PRIVATE3:
            case SDL_EVENT_POLL_SENTINEL:
            case SDL_EVENT_USER:
            case SDL_EVENT_LAST:
            case SDL_EVENT_ENUM_PADDING:
                break;
        }
    }
}

} // namespace sf::priv
