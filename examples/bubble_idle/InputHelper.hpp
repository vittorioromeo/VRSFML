#pragma once

#include "InputState.hpp"

#include "SFML/Window/Event.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"


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
    void applyEvent(const sf::Event& event)
    {
        m_currentInputState.apply(event);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isKeyDown(const sf::Keyboard::Key key) const
    {
        return m_currentInputState.keys[static_cast<sf::base::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasKeyJustPressed(const sf::Keyboard::Key key) const
    {
        return m_currentInputState.keys[static_cast<sf::base::SizeT>(key)] &&
               !m_previousInputState.keys[static_cast<sf::base::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasKeyJustReleased(const sf::Keyboard::Key key) const
    {
        return !m_currentInputState.keys[static_cast<sf::base::SizeT>(key)] &&
               m_previousInputState.keys[static_cast<sf::base::SizeT>(key)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool isMouseButtonDown(const sf::Mouse::Button button) const
    {
        return m_currentInputState.buttons[static_cast<sf::base::SizeT>(button)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasMouseButtonJustPressed(const sf::Mouse::Button button) const
    {
        return m_currentInputState.buttons[static_cast<sf::base::SizeT>(button)] &&
               !m_previousInputState.buttons[static_cast<sf::base::SizeT>(button)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] bool wasMouseButtonJustReleased(const sf::Mouse::Button button) const
    {
        return !m_currentInputState.buttons[static_cast<sf::base::SizeT>(button)] &&
               m_previousInputState.buttons[static_cast<sf::base::SizeT>(button)];
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
    [[nodiscard, gnu::pure]] sf::Vector2i getFingerPosition(const unsigned int finger) const
    {
        SFML_BASE_ASSERT(m_currentInputState.fingerPositions[finger].hasValue());
        return *m_currentInputState.fingerPositions[finger];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] sf::Vector2i getMousePosition() const
    {
        return m_currentInputState.mousePosition;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] sf::Vector2i getMousePositionDelta() const
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
    [[nodiscard, gnu::pure]] sf::base::Optional<unsigned int> getFirstNewlyPressedFinger() const
    {
        for (unsigned int i = 0; i < m_currentInputState.fingerPositions.size(); ++i)
        {
            if (wasFingerJustPressed(i))
                return sf::base::makeOptional<unsigned int>(i);
        }

        return sf::base::nullOpt;
    }
};
