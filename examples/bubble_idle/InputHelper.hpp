#pragma once

#include "InputState.hpp"

#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
{
class Event;
}


////////////////////////////////////////////////////////////
class InputHelper
{
private:
    InputState m_currentInputState{};
    InputState m_previousInputState{};

public:
    ////////////////////////////////////////////////////////////
    void beginNewFrame()
    {
        m_previousInputState = m_currentInputState;
        // Intentionally not resetting `m_currentInputState`
    }

    ////////////////////////////////////////////////////////////
    void applyEvent(const za::Event& event);

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isKeyDown(const za::Keyboard::Key key) const
    {
        return m_currentInputState.keys[static_cast<za::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasKeyJustPressed(const za::Keyboard::Key key) const
    {
        return m_currentInputState.keys[static_cast<za::SizeT>(key)] &&
               !m_previousInputState.keys[static_cast<za::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasKeyJustReleased(const za::Keyboard::Key key) const
    {
        return !m_currentInputState.keys[static_cast<za::SizeT>(key)] &&
               m_previousInputState.keys[static_cast<za::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isMouseButtonDown(const za::Mouse::Button button) const
    {
        return m_currentInputState.buttons[static_cast<za::SizeT>(button)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasMouseButtonJustPressed(const za::Mouse::Button button) const
    {
        return m_currentInputState.buttons[static_cast<za::SizeT>(button)] &&
               !m_previousInputState.buttons[static_cast<za::SizeT>(button)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasMouseButtonJustReleased(const za::Mouse::Button button) const
    {
        return !m_currentInputState.buttons[static_cast<za::SizeT>(button)] &&
               m_previousInputState.buttons[static_cast<za::SizeT>(button)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isFingerDown(const unsigned int finger) const
    {
        return m_currentInputState.fingerPositions[finger].hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasFingerJustPressed(const unsigned int finger) const
    {
        return m_currentInputState.fingerPositions[finger].hasValue() &&
               !m_previousInputState.fingerPositions[finger].hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasFingerJustReleased(const unsigned int finger) const
    {
        return !m_currentInputState.fingerPositions[finger].hasValue() &&
               m_previousInputState.fingerPositions[finger].hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::Vec2i getFingerPosition(const unsigned int finger) const
    {
        ZA_ASSERT(m_currentInputState.fingerPositions[finger].hasValue());
        return *m_currentInputState.fingerPositions[finger];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::Vec2i getMousePosition() const
    {
        return m_currentInputState.mousePosition;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::Vec2i getMousePositionDelta() const
    {
        return m_currentInputState.mousePosition - m_previousInputState.mousePosition;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isAltDown() const
    {
        return m_currentInputState.alt;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isControlDown() const
    {
        return m_currentInputState.control;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isShiftDown() const
    {
        return m_currentInputState.shift;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isSystemDown() const
    {
        return m_currentInputState.system;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::Optional<unsigned int> getFirstNewlyPressedFinger() const
    {
        for (unsigned int i = 0; i < m_currentInputState.fingerPositions.size(); ++i)
        {
            if (wasFingerJustPressed(i))
                return za::makeOptional<unsigned int>(i);
        }

        return za::nullOpt;
    }
};
