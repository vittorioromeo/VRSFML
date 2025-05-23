#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec3.hpp"

#include <android/sensor.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Android implementation of sensors
///
////////////////////////////////////////////////////////////
class SensorImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the sensor module
    ///
    ////////////////////////////////////////////////////////////
    static void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the sensor module
    ///
    ////////////////////////////////////////////////////////////
    static void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a sensor is available
    ///
    /// \param sensor Sensor to check
    ///
    /// \return `true` if the sensor is available, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isAvailable(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Open the sensor
    ///
    /// \param sensor Type of the sensor
    ///
    /// \return `true` on success, `false` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Close the sensor
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    /// \brief Update the sensor and get its new value
    ///
    /// \return Sensor value
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec3f update() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable the sensor
    ///
    /// \param enabled `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setEnabled(bool enabled);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get the default Android sensor matching the sensor type
    ///
    /// \param type Type of the sensor
    ///
    /// \return The default Android sensor, `nullptr` otherwise
    ///
    ////////////////////////////////////////////////////////////
    static const ASensor* getDefaultSensor(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Process the pending sensor data available and add them to our lists
    ///
    /// \param fd     File descriptor
    /// \param events Bitmask of the poll events that were triggered
    /// \param data   Data pointer supplied
    ///
    /// \return Whether it should continue (1) or unregister the callback (0)
    ///
    ////////////////////////////////////////////////////////////
    static int processSensorEvents(int fd, int events, void* data);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const ASensor* m_sensor; ///< Android sensor structure
    Sensor::Type   m_type;   ///< Type of the sensor
};

} // namespace sf::priv
