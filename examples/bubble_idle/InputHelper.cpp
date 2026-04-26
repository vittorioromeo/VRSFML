#include "InputHelper.hpp"


////////////////////////////////////////////////////////////
void InputHelper::applyEvent(const sf::Event& event)
{
    m_currentInputState.apply(event);
}
