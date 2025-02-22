#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/JoystickIdentification.hpp"
#include "SFML/Window/JoystickImpl.hpp"
#include "SFML/Window/JoystickManager.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct JoystickManager::Impl
{
    JoystickImpl           impls[Joystick::MaxCount];           //!< Joystick implementations
    JoystickState          states[Joystick::MaxCount];          //!< Joystick states
    JoystickCapabilities   capabilities[Joystick::MaxCount];    //!< Joystick capabilities
    JoystickIdentification identifications[Joystick::MaxCount]; //!< Joystick identifications
};


////////////////////////////////////////////////////////////
const JoystickCapabilities& JoystickManager::getCapabilities(unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount && "Joystick index must be less than `Joystick::MaxCount`");
    return m_impl->capabilities[joystickId];
}


////////////////////////////////////////////////////////////
const JoystickState& JoystickManager::getState(unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount && "Joystick index must be less than `Joystick::MaxCount`");
    return m_impl->states[joystickId];
}


////////////////////////////////////////////////////////////
const JoystickIdentification& JoystickManager::getIdentification(unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount && "Joystick index must be less than `Joystick::MaxCount`");
    return m_impl->identifications[joystickId];
}


////////////////////////////////////////////////////////////
void JoystickManager::update()
{
    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
    {
        auto& [impls, states, capabilities, identifications] = *m_impl;

        if (states[i].connected)
        {
            // Get the current state of the joystick
            states[i] = impls[i].update();

            // Check if it's still connected
            if (!states[i].connected)
            {
                impls[i].close();

                capabilities[i]    = {};
                states[i]          = {};
                identifications[i] = {};
            }
        }
        else
        {
            // Check if the joystick was connected since last update
            if (JoystickImpl::isConnected(i) && impls[i].open(i))
            {
                capabilities[i]    = impls[i].getCapabilities();
                states[i]          = impls[i].update();
                identifications[i] = impls[i].getIdentification();
            }
        }
    }
}


////////////////////////////////////////////////////////////
JoystickManager::JoystickManager()
{
    JoystickImpl::initialize();
}


////////////////////////////////////////////////////////////
JoystickManager::~JoystickManager()
{
    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
        if (m_impl->states[i].connected)
            m_impl->impls[i].close();

    JoystickImpl::cleanup();
}

} // namespace sf::priv
