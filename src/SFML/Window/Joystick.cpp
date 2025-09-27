// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/JoystickCapabilities.hpp"
#include "SFML/Window/JoystickIdentification.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/JoystickState.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
void Joystick::update()
{
    WindowContext::getJoystickManager().update();
}


////////////////////////////////////////////////////////////
Joystick::Query::Query(const priv::JoystickManager& joystickManager, unsigned int joystickId) :
    m_joystickManager(joystickManager),
    m_joystickId(joystickId)
{
    SFML_BASE_ASSERT(isConnected() && "Created query for unconnected joystick");
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getIndex() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get index");
    return m_joystickId;
}


////////////////////////////////////////////////////////////
const String& Joystick::Query::getName() const
{
    return getIdentification().name;
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getVendorId() const
{
    return getIdentification().vendorId;
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getProductId() const
{
    return getIdentification().productId;
}


////////////////////////////////////////////////////////////
const priv::JoystickIdentification& Joystick::Query::getIdentification() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get identification");
    return m_joystickManager.getIdentification(m_joystickId);
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getButtonCount() const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get button count");
    return m_joystickManager.getCapabilities(m_joystickId).buttonCount;
}


////////////////////////////////////////////////////////////
bool Joystick::Query::hasAxis(Axis axis) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: has axis");
    return m_joystickManager.getCapabilities(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Joystick::Query::isButtonPressed(unsigned int button) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: is button pressed");
    return m_joystickManager.getState(m_joystickId).buttons[button];
}


////////////////////////////////////////////////////////////
float Joystick::Query::getAxisPosition(Axis axis) const
{
    SFML_BASE_ASSERT(isConnected() && "Query on unconnected joystick: get axis position");
    return m_joystickManager.getState(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Joystick::Query::isConnected() const
{
    return m_joystickManager.isConnected(m_joystickId);
}


////////////////////////////////////////////////////////////
base::Optional<Joystick::Query> Joystick::query(unsigned int joystickId)
{
    const auto& joystickManager = WindowContext::getJoystickManager();

    if (!joystickManager.isConnected(joystickId))
        return base::nullOpt;

    return base::makeOptionalFromFunc([&] { return Query{joystickManager, joystickId}; });
}

} // namespace sf
