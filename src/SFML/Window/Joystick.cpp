#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/JoystickIdentification.hpp"
#include "SFML/Window/JoystickImpl.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] bool isConnectedImpl(const sf::priv::JoystickManager& joystickManager, unsigned int joystickId)
{
    return joystickManager.getState(joystickId).connected;
}

} // namespace


namespace sf::Joystick
{
////////////////////////////////////////////////////////////
void update(WindowContext& windowContext)
{
    windowContext.getJoystickManager().update();
}


////////////////////////////////////////////////////////////
Query::Query(const priv::JoystickManager& joystickManager, unsigned int joystickId) :
m_joystickManager(joystickManager),
m_joystickId(joystickId)
{
    SFML_BASE_ASSERT(isConnected() && "Created query for unconnected joystick");
}


////////////////////////////////////////////////////////////
unsigned int Query::getIndex() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get index");
    return m_joystickId;
}


////////////////////////////////////////////////////////////
const String& Query::getName() const
{
    return getIdentification().name;
}


////////////////////////////////////////////////////////////
unsigned int Query::getVendorId() const
{
    return getIdentification().vendorId;
}


////////////////////////////////////////////////////////////
unsigned int Query::getProductId() const
{
    return getIdentification().productId;
}


////////////////////////////////////////////////////////////
const priv::JoystickIdentification& Query::getIdentification() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get identification");
    return m_joystickManager.getIdentification(m_joystickId);
}


////////////////////////////////////////////////////////////
unsigned int Query::getButtonCount() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get button count");
    return m_joystickManager.getCapabilities(m_joystickId).buttonCount;
}


////////////////////////////////////////////////////////////
bool Query::hasAxis(Axis axis) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: has axis");
    return m_joystickManager.getCapabilities(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Query::isButtonPressed(unsigned int button) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: is button pressed");
    return m_joystickManager.getState(m_joystickId).buttons[button];
}


////////////////////////////////////////////////////////////
float Query::getAxisPosition(Axis axis) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get axis position");
    return m_joystickManager.getState(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Query::isConnected() const
{
    return isConnectedImpl(m_joystickManager, m_joystickId);
}


////////////////////////////////////////////////////////////
base::Optional<Query> query(const WindowContext& windowContext, unsigned int joystickId)
{
    const auto& joystickManager = windowContext.getJoystickManager();

    if (!isConnectedImpl(joystickManager, joystickId))
        return base::nullOpt;

    return base::makeOptionalFromFunc([&] { return Query{joystickManager, joystickId}; });
}

} // namespace sf::Joystick
