#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/JoystickImpl.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/Sensor.hpp"
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowImplType.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/EnumArray.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <queue>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
// Yes, this is a rather weird namespace.
namespace WindowImplImpl
{
const sf::priv::WindowImpl* fullscreenWindow = nullptr;
} // namespace WindowImplImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct WindowImpl::Impl
{
    std::queue<Event> events;                                           //!< Queue of available events
    JoystickState     joystickStates[Joystick::MaxCount]{};             //!< Previous state of the joysticks
    base::EnumArray<Sensor::Type, Vector3f, Sensor::Count> sensorValue; //!< Previous value of the sensors
    float joystickThreshold{0.1f}; //!< Joystick threshold (minimum motion for "move" event to be generated)
    base::EnumArray<Joystick::Axis, float, Joystick::AxisCount>
        previousAxes[Joystick::MaxCount]{}; //!< Position of each axis last time a move event triggered, in range [-100, 100]
    base::Optional<Vector2u> minimumSize; //!< Minimum window size
    base::Optional<Vector2u> maximumSize; //!< Maximum window size
};


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

    auto windowImpl = base::makeUnique<WindowImplType>(windowSettings);

    if (windowSettings.fullscreen)
        WindowImplImpl::fullscreenWindow = windowImpl.get();

    return windowImpl;
}


////////////////////////////////////////////////////////////
base::UniquePtr<WindowImpl> WindowImpl::create(WindowHandle handle)
{
    return base::makeUnique<WindowImplType>(handle);
}


////////////////////////////////////////////////////////////
WindowImpl::WindowImpl()
{
    auto& joystickManager = WindowContext::getJoystickManager();

    // Get the initial joystick states
    joystickManager.update();

    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
    {
        m_impl->joystickStates[i] = joystickManager.getState(i);
        m_impl->previousAxes[i].fill(0.f);
    }

    // Get the initial sensor states
    for (Vector3f& vec : m_impl->sensorValue.data)
        vec = Vector3f(0, 0, 0);
}


////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl()
{
    if (WindowImplImpl::fullscreenWindow == this)
        WindowImplImpl::fullscreenWindow = nullptr;
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
        const bool infiniteTimeout = timeout == Time::Zero;
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

        // Connection state
        const bool connected = m_impl->joystickStates[i].connected;
        if (previousState.connected ^ connected)
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

            if (prevPressed ^ currPressed)
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
    processEvents();
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

} // namespace sf::priv
