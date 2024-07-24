#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Emscripten/JoystickImpl.hpp>
#include <SFML/Window/Event.hpp>

#include <SFML/System/Err.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cmath>


namespace
{
////////////////////////////////////////////////////////////
void updatePluggedList()
{
    int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
    {
        for (int i = 0; i < sf::Joystick::Count; ++i)
        {
            joysticksConnected[i] = false;
        }

        return;
    }

    for (int i = 0; (i < sf::Joystick::Count) && (i < numJoysticks); ++i)
    {
        EmscriptenGamepadEvent gamepadEvent;
        if (emscripten_get_gamepad_status(i, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
        {
            sf::err() << "Failed to get status of gamepad " << i << std::endl;
            joysticksConnected[i] = false;
            continue;
        }

        if (gamepadEvent.connected)
            joysticksConnected[i] = true;
        else
            joysticksConnected[i] = false;
    }
}


////////////////////////////////////////////////////////////
EM_BOOL gamepadCallback(int eventType, const EmscriptenGamepadEvent* e, void* userData)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_GAMEPADCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            if (window)
            {
                for (int i = 0; i < sf::Joystick::Count; ++i)
                {
                    if (!previousConnected[i] && joysticksConnected[i])
                    {
                        sf::Event event;
                        event.type                       = sf::Event::JoystickConnected;
                        event.joystickConnect.joystickId = i;
                        window->pushEvent(event);
                    }
                }
            }

            return 1;
        }
        case EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            if (window)
            {
                for (int i = 0; i < sf::Joystick::Count; ++i)
                {
                    if (previousConnected[i] && !joysticksConnected[i])
                    {
                        sf::Event event;
                        event.type                       = sf::Event::JoystickDisconnected;
                        event.joystickConnect.joystickId = i;
                        window->pushEvent(event);
                    }
                }
            }

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

} //


////////////////////////////////////////////////////////////
void JoystickImpl::initialize()
{
    static bool callbacksSet = false;

    if (callbacksSet)
        return;

    if (emscripten_set_gamepadconnected_callback(0, 1, gamepadCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::err() << "Failed to set gamepadconnected callback" << std::endl;

    if (emscripten_set_gamepaddisconnected_callback(0, 1, gamepadCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::err() << "Failed to set gamepaddisconnected callback" << std::endl;

    callbacksSet = true;
}


////////////////////////////////////////////////////////////
void JoystickImpl::cleanup()
{
}


////////////////////////////////////////////////////////////
bool JoystickImpl::isConnected(unsigned int index)
{
    return joysticksConnected[index];
}


////////////////////////////////////////////////////////////
bool JoystickImpl::open(unsigned int index)
{
    if (!isConnected(index))
        return false;

    int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
        return false;

    if (index >= numJoysticks)
        return false;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::err() << "Failed to get status of gamepad " << index << std::endl;
        joysticksConnected[index] = false;
        return false;
    }

    if (!gamepadEvent.connected)
    {
        joysticksConnected[index] = false;
        return false;
    }

    m_index = index;

    m_identification.name      = String::fromUtf8(gamepadEvent.id, gamepadEvent.id + 64);
    m_identification.vendorId  = 0;
    m_identification.productId = 0;

    return true;
}


////////////////////////////////////////////////////////////
void JoystickImpl::close()
{
    m_index = 0;
}


////////////////////////////////////////////////////////////
JoystickCaps JoystickImpl::getCapabilities() const
{
    JoystickCaps caps;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(m_index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::err() << "Failed to get status of gamepad " << m_index << std::endl;
        joysticksConnected[m_index] = false;
        return caps;
    }

    // Get the number of buttons
    caps.buttonCount = gamepadEvent.numButtons;

    if (caps.buttonCount > Joystick::ButtonCount)
        caps.buttonCount = Joystick::ButtonCount;

    // Only support the "standard" mapping for now
    if (std::strcmp(gamepadEvent.mapping, "standard") == 0)
    {
        caps.axes[Joystick::X]    = true;
        caps.axes[Joystick::Y]    = true;
        caps.axes[Joystick::Z]    = false;
        caps.axes[Joystick::R]    = true;
        caps.axes[Joystick::U]    = true;
        caps.axes[Joystick::V]    = false;
        caps.axes[Joystick::PovX] = false;
        caps.axes[Joystick::PovY] = false;
    }
    else
    {
        caps.axes[Joystick::X]    = false;
        caps.axes[Joystick::Y]    = false;
        caps.axes[Joystick::Z]    = false;
        caps.axes[Joystick::R]    = false;
        caps.axes[Joystick::U]    = false;
        caps.axes[Joystick::V]    = false;
        caps.axes[Joystick::PovX] = false;
        caps.axes[Joystick::PovY] = false;
    }

    return caps;
}


////////////////////////////////////////////////////////////
Joystick::Identification JoystickImpl::getIdentification() const
{
    return m_identification;
}


////////////////////////////////////////////////////////////
JoystickState JoystickImpl::update()
{
    JoystickState state;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(m_index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::err() << "Failed to get status of gamepad " << m_index << std::endl;
        joysticksConnected[m_index] = false;
        return state;
    }

    for (int i = 0; (i < gamepadEvent.numButtons) && (i < Joystick::ButtonCount); ++i)
    {
        state.buttons[i] = gamepadEvent.digitalButton[i];
    }

    if (std::strcmp(gamepadEvent.mapping, "standard") == 0)
    {
        state.axes[Joystick::X] = static_cast<float>(gamepadEvent.axis[0] * 100.0);
        state.axes[Joystick::Y] = static_cast<float>(gamepadEvent.axis[1] * 100.0);
        state.axes[Joystick::R] = static_cast<float>(gamepadEvent.axis[2] * 100.0);
        state.axes[Joystick::U] = static_cast<float>(gamepadEvent.axis[3] * 100.0);
    }

    return state;
}


////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    VideoMode desktop = getDesktopMode();

    std::vector<VideoMode> modes;
    modes.push_back(desktop);
    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    int width  = emscripten_run_script_int("screen.width");
    int height = emscripten_run_script_int("screen.height");
    return VideoMode(width, height);
}

} // namespace priv
} // namespace sf
