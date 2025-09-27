// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SensorImpl.hpp"
#include "SFML/Window/iOS/SFAppDelegate.hpp"

#include "SFML/System/Angle.hpp"


namespace
{
unsigned int deviceMotionEnabledCount = 0;
}


namespace sf::priv
{
////////////////////////////////////////////////////////////
void SensorImpl::initialize()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
void SensorImpl::cleanup()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
bool SensorImpl::isAvailable(Sensor::Type sensor)
{
    switch (sensor)
    {
        case Sensor::Type::Accelerometer:
            return [SFAppDelegate getInstance].motionManager.accelerometerAvailable;

        case Sensor::Type::Gyroscope:
            return [SFAppDelegate getInstance].motionManager.gyroAvailable;

        case Sensor::Type::Magnetometer:
            return [SFAppDelegate getInstance].motionManager.magnetometerAvailable;

        case Sensor::Type::Gravity:
        case Sensor::Type::UserAcceleration:
        case Sensor::Type::Orientation:
            return [SFAppDelegate getInstance].motionManager.deviceMotionAvailable;

        default:
            return false;
    }
}


////////////////////////////////////////////////////////////
bool SensorImpl::open(Sensor::Type sensor)
{
    // Store the sensor type
    m_sensor = sensor;

    // The sensor is disabled by default
    m_enabled = false;

    // Set the refresh rate (use the maximum allowed)
    constexpr NSTimeInterval updateInterval = 1. / 60.;
    switch (sensor)
    {
        case Sensor::Type::Accelerometer:
            [SFAppDelegate getInstance].motionManager.accelerometerUpdateInterval = updateInterval;
            break;

        case Sensor::Type::Gyroscope:
            [SFAppDelegate getInstance].motionManager.gyroUpdateInterval = updateInterval;
            break;

        case Sensor::Type::Magnetometer:
            [SFAppDelegate getInstance].motionManager.magnetometerUpdateInterval = updateInterval;
            break;

        case Sensor::Type::Gravity:
        case Sensor::Type::UserAcceleration:
        case Sensor::Type::Orientation:
            [SFAppDelegate getInstance].motionManager.deviceMotionUpdateInterval = updateInterval;
            break;

        default:
            break;
    }

    return true;
}


////////////////////////////////////////////////////////////
void SensorImpl::close()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
Vec3f SensorImpl::update()
{
    CMMotionManager* const manager = [SFAppDelegate getInstance].motionManager;

    switch (m_sensor)
    {
        case Sensor::Type::Accelerometer:
            // Acceleration is given in G, convert to m/s^2
            return Vec3f(9.81 * Vec3(manager.accelerometerData.acceleration.x,
                                           manager.accelerometerData.acceleration.y,
                                           manager.accelerometerData.acceleration.z));

        case Sensor::Type::Gyroscope:
            // Rotation rates are given in rad/s
            return Vec3f(
                Vec3(manager.gyroData.rotationRate.x, manager.gyroData.rotationRate.y, manager.gyroData.rotationRate.z));

        case Sensor::Type::Magnetometer:
            // Magnetic field is given in microteslas
            return Vec3f(Vec3(manager.magnetometerData.magneticField.x,
                                    manager.magnetometerData.magneticField.y,
                                    manager.magnetometerData.magneticField.z));

        case Sensor::Type::UserAcceleration:
            // User acceleration is given in G, convert to m/s^2
            return Vec3f(9.81 * Vec3(manager.deviceMotion.userAcceleration.x,
                                           manager.deviceMotion.userAcceleration.y,
                                           manager.deviceMotion.userAcceleration.z));

        case Sensor::Type::Orientation:
            // Absolute rotation (Euler) angles are given in radians
            return Vec3f(Vec3<double>(manager.deviceMotion.attitude.yaw,
                                            manager.deviceMotion.attitude.pitch,
                                            manager.deviceMotion.attitude.roll));

        default:
            return {};
    }
}


////////////////////////////////////////////////////////////
void SensorImpl::setEnabled(bool enabled)
{
    // Don't do anything if the state is the same
    if (enabled == m_enabled)
        return;

    switch (m_sensor)
    {
        case Sensor::Type::Accelerometer:
            if (enabled)
                [[SFAppDelegate getInstance].motionManager startAccelerometerUpdates];
            else
                [[SFAppDelegate getInstance].motionManager stopAccelerometerUpdates];
            break;

        case Sensor::Type::Gyroscope:
            if (enabled)
                [[SFAppDelegate getInstance].motionManager startGyroUpdates];
            else
                [[SFAppDelegate getInstance].motionManager stopGyroUpdates];
            break;

        case Sensor::Type::Magnetometer:
            if (enabled)
                [[SFAppDelegate getInstance].motionManager startMagnetometerUpdates];
            else
                [[SFAppDelegate getInstance].motionManager stopMagnetometerUpdates];
            break;

        case Sensor::Type::Gravity:
        case Sensor::Type::UserAcceleration:
        case Sensor::Type::Orientation:
            // these 3 sensors all share the same implementation, so we must disable
            // it only if the three sensors are disabled
            if (enabled)
            {
                if (deviceMotionEnabledCount == 0)
                    [[SFAppDelegate getInstance].motionManager startDeviceMotionUpdates];
                ++deviceMotionEnabledCount;
            }
            else
            {
                --deviceMotionEnabledCount;
                if (deviceMotionEnabledCount == 0)
                    [[SFAppDelegate getInstance].motionManager stopDeviceMotionUpdates];
            }
            break;

        default:
            break;
    }

    // Update the enable state
    m_enabled = enabled;
}

} // namespace sf::priv
