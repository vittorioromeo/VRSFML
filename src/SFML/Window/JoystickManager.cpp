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
/// \brief Joystick information and state
///
////////////////////////////////////////////////////////////
struct Item
{
    JoystickImpl             joystick;       //!< Joystick implementation
    JoystickState            state;          //!< The current joystick state
    JoystickCaps             capabilities;   //!< The joystick capabilities
    Joystick::Identification identification; //!< The joystick identification
};


////////////////////////////////////////////////////////////
struct JoystickManager::Impl
{
    Item joysticks[Joystick::Count]; //!< Joysticks information and state
};


////////////////////////////////////////////////////////////
JoystickManager& JoystickManager::getInstance()
{
    static JoystickManager instance;
    return instance;
}


////////////////////////////////////////////////////////////
const JoystickCaps& JoystickManager::getCapabilities(unsigned int joystick) const
{
    SFML_BASE_ASSERT(joystick < Joystick::Count && "Joystick index must be less than Joystick::Count");
    return m_impl->joysticks[joystick].capabilities;
}


////////////////////////////////////////////////////////////
const JoystickState& JoystickManager::getState(unsigned int joystick) const
{
    SFML_BASE_ASSERT(joystick < Joystick::Count && "Joystick index must be less than Joystick::Count");
    return m_impl->joysticks[joystick].state;
}


////////////////////////////////////////////////////////////
const Joystick::Identification& JoystickManager::getIdentification(unsigned int joystick) const
{
    SFML_BASE_ASSERT(joystick < Joystick::Count && "Joystick index must be less than Joystick::Count");
    return m_impl->joysticks[joystick].identification;
}


////////////////////////////////////////////////////////////
void JoystickManager::update()
{
    for (unsigned int i = 0; i < Joystick::Count; ++i)
    {
        Item& item = m_impl->joysticks[i];

        if (item.state.connected)
        {
            // Get the current state of the joystick
            item.state = item.joystick.update();

            // Check if it's still connected
            if (!item.state.connected)
            {
                item.joystick.close();
                item.capabilities   = JoystickCaps();
                item.state          = JoystickState();
                item.identification = Joystick::Identification();
            }
        }
        else
        {
            // Check if the joystick was connected since last update
            if (JoystickImpl::isConnected(i))
            {
                if (item.joystick.open(i))
                {
                    item.capabilities   = item.joystick.getCapabilities();
                    item.state          = item.joystick.update();
                    item.identification = item.joystick.getIdentification();
                }
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
    for (Item& item : m_impl->joysticks)
    {
        if (item.state.connected)
            item.joystick.close();
    }

    JoystickImpl::cleanup();
}

} // namespace sf::priv
