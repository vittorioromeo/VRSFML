// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Joystick.hpp"

#include "Zancle/Window/JoystickCapabilities.hpp"
#include "Zancle/Window/JoystickIdentification.hpp"
#include "Zancle/Window/JoystickManager.hpp"
#include "Zancle/Window/JoystickState.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Diagnostic/Assert.hpp"


namespace za
{
////////////////////////////////////////////////////////////
void Joystick::update()
{
    WindowContext::getJoystickManager().update();
}


////////////////////////////////////////////////////////////
Joystick::Query::Query(const priv::JoystickManager& joystickManager, const unsigned int joystickId) :
    m_joystickManager(joystickManager),
    m_joystickId(joystickId)
{
    ZA_ASSERT(isConnected() && "Created query for unconnected joystick");
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getIndex() const
{
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: get index");
    return m_joystickId;
}


////////////////////////////////////////////////////////////
const Utf8String& Joystick::Query::getName() const
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
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: get identification");
    return m_joystickManager.getIdentification(m_joystickId);
}


////////////////////////////////////////////////////////////
unsigned int Joystick::Query::getButtonCount() const
{
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: get button count");
    return m_joystickManager.getCapabilities(m_joystickId).buttonCount;
}


////////////////////////////////////////////////////////////
bool Joystick::Query::hasAxis(const Axis axis) const
{
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: has axis");
    return m_joystickManager.getCapabilities(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Joystick::Query::isButtonPressed(const unsigned int button) const
{
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: is button pressed");
    return m_joystickManager.getState(m_joystickId).buttons[button];
}


////////////////////////////////////////////////////////////
float Joystick::Query::getAxisPosition(const Axis axis) const
{
    ZA_ASSERT(isConnected() && "Query on unconnected joystick: get axis position");
    return m_joystickManager.getState(m_joystickId).axes[axis];
}


////////////////////////////////////////////////////////////
bool Joystick::Query::isConnected() const
{
    return m_joystickManager.isConnected(m_joystickId);
}


////////////////////////////////////////////////////////////
za::Optional<Joystick::Query> Joystick::query(const unsigned int joystickId)
{
    if (joystickId >= Joystick::MaxCount)
    {
        priv::errMsg("Invalid joystick index: {}, valid range is [0 .. {}]", joystickId, (Joystick::MaxCount - 1));

        return za::nullOpt;
    }

    const auto& joystickManager = WindowContext::getJoystickManager();

    if (!joystickManager.isConnected(joystickId))
        return za::nullOpt;

    return za::makeOptionalFromFunc([&] { return Query{joystickManager, joystickId}; });
}

} // namespace za
