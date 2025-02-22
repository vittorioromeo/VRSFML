#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <array>
#include <unordered_map>
#include <vector>

#include <CoreFoundation/CoreFoundation.h>

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief macOS implementation of joysticks
///
////////////////////////////////////////////////////////////
class JoystickImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick is currently connected
    ///
    /// \param index Index of the joystick to check
    ///
    /// \return `true` if the joystick is connected, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isConnected(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Open the joystick
    ///
    /// \param index Index assigned to the joystick
    ///
    /// \return `true` on success, `false` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Close the joystick
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    /// \brief Get the joystick capabilities
    ///
    /// \return Joystick capabilities
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickCapabilities getCapabilities() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the joystick identification
    ///
    /// \return Joystick identification
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const JoystickIdentification& getIdentification() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the joystick and get its new state
    ///
    /// \return Joystick state
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickState update();

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    using Location      = long;
    using AxisMap       = std::unordered_map<Joystick::Axis, IOHIDElementRef>;
    using ButtonsVector = std::vector<IOHIDElementRef>;

    AxisMap                m_axis;           ///< Axes (but not POV/Hat) of the joystick
    IOHIDElementRef        m_hat{};          ///< POV/Hat axis of the joystick
    ButtonsVector          m_buttons;        ///< Buttons of the joystick
    unsigned int           m_index{};        ///< SFML index
    JoystickIdentification m_identification; ///< Joystick identification

    // NOLINTNEXTLINE(readability-identifier-naming)
    static inline std::array<Location, Joystick::MaxCount> m_locationIDs{}; ///< Global Joystick register
    /// For a corresponding SFML index, m_locationIDs is either some USB
    /// location or 0 if there isn't currently a connected joystick device
};

} // namespace sf::priv
