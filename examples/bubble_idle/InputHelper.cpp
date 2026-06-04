#include "InputHelper.hpp"


////////////////////////////////////////////////////////////
void InputHelper::applyEvent(const za::Event& event)
{
    m_currentInputState.apply(event);
}
