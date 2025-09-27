#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class String;
} // namespace sf


namespace sf::priv
{
class JoystickManager;
struct JoystickIdentification;
} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Give access to the real-time state of the joysticks
///
////////////////////////////////////////////////////////////
struct Joystick
{
    ////////////////////////////////////////////////////////////
    /// \brief Axes supported by SFML joysticks
    ///
    ////////////////////////////////////////////////////////////
    enum class Axis
    {
        X,    //!< The X axis
        Y,    //!< The Y axis
        Z,    //!< The Z axis
        R,    //!< The R axis
        U,    //!< The U axis
        V,    //!< The V axis
        PovX, //!< The X axis of the point-of-view hat
        PovY  //!< The Y axis of the point-of-view hat
    };

    ////////////////////////////////////////////////////////////
    /// \brief Constants related to joysticks capabilities
    ///
    ////////////////////////////////////////////////////////////
    enum : unsigned int
    {
        MaxCount    = 8u,  //!< Maximum number of supported joysticks
        ButtonCount = 32u, //!< Maximum number of supported buttons
        AxisCount   = 8u   //!< Maximum number of supported axes
    };

    ////////////////////////////////////////////////////////////
    /// \brief Update the states of all joysticks
    ///
    /// This function is used internally by SFML, so you normally
    /// don't have to call it explicitly. However, you may need to
    /// call it if you have no window yet (or no window at all):
    /// in this case the joystick states are not updated automatically.
    ///
    ////////////////////////////////////////////////////////////
    SFML_WINDOW_API static void update();

    // Forward declaration for friendship
    class Query;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    /// \param joystick Index of the joystick to check
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Query> query(unsigned int joystickId);
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_WINDOW_API Joystick::Query
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Return the index of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getIndex() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the name of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const String& getName() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the manufacturer identifier of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getVendorId() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the product identifier of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getProductId() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the number of buttons supported by a joystick
    ///
    /// \return Number of buttons supported by the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getButtonCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick supports a given axis
    ///
    /// \param axis Axis to check
    ///
    /// \return `true` if the joystick supports the axis, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasAxis(Axis axis) const;

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick button is pressed
    ///
    /// \param button Button to check
    ///
    /// \return `true` if the button is pressed, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isButtonPressed(unsigned int button) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current position of a joystick axis
    ///
    /// \param axis Axis to check
    ///
    /// \return Current position of the axis, in range [-100 .. 100]
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getAxisPosition(Axis axis) const;

private:
    friend base::Optional<Query> Joystick::query(unsigned int joystickId);

    ////////////////////////////////////////////////////////////
    /// \brief Create a joystick state query
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Query(const priv::JoystickManager& joystickManager, unsigned int joystickId);

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick is connected
    ///
    /// \return `true` if the joystick is connected, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isConnected() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the identification of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const priv::JoystickIdentification& getIdentification() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const priv::JoystickManager& m_joystickManager;
    unsigned int                 m_joystickId;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \namespace sf::Joystick
/// \ingroup window
///
/// `sf::Joystick` provides an interface to the state of the
/// joysticks. Each joystick is identified by an index that
/// is passed to the functions in this namespace.
///
/// This namespace allows users to query the state of joysticks at any
/// time and directly, without having to deal with a window and
/// its events. Compared to the `JoystickMoved`, `JoystickButtonPressed`
/// and `JoystickButtonReleased` events, `sf::Joystick` can retrieve the
/// state of axes and buttons of joysticks at any time
/// (you don't need to store and update a boolean on your side
/// in order to know if a button is pressed or released), and you
/// always get the real state of joysticks, even if they are
/// moved, pressed or released when your window is out of focus
/// and no event is triggered.
///
/// SFML supports:
/// \li 8 joysticks (`sf::Joystick::MaxCount`)
/// \li 32 buttons per joystick (`sf::Joystick::ButtonCount`)
/// \li 8 axes per joystick (`sf::Joystick::AxisCount`)
///
/// Unlike the keyboard or mouse, the state of joysticks is sometimes
/// not directly available (depending on the OS), therefore an `update()`
/// function must be called in order to update the current state of
/// joysticks. When you have a window with event handling, this is done
/// automatically, you don't need to call anything. But if you have no
/// window, or if you want to check joysticks state before creating one,
/// you must call `sf::Joystick::update` explicitly.
///
/// Usage example:
/// \code
/// const auto query = sf::Joystick::query(0);
///
/// if (!query.hasValue()) // Is joystick #0 disconnected?
///     return;
///
/// // How many buttons does joystick #0 support?
/// const unsigned int buttonCount = query.getButtonCount();
///
/// // Does joystick #0 define a X axis?
/// const bool hasX = query.hasAxis(sf::Joystick::Axis::X);
///
/// // Is button #2 pressed on joystick #0?
/// const bool pressed = query.isButtonPressed(2);
///
/// // What's the current position of the Y axis on joystick #0?
/// const float position = query.getAxisPosition(sf::Joystick::Axis::Y);
/// \endcode
///
/// \see `sf::Keyboard`, `sf::Mouse`
///
////////////////////////////////////////////////////////////
