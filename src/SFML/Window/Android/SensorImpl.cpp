// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SensorImpl.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/EnumArray.hpp"
#include "SFML/Base/Optional.hpp"

#include <android/looper.h>

#if defined(__clang__) || defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// Define missing constants
#define ASENSOR_TYPE_GRAVITY             0x00'00'00'09
#define ASENSOR_TYPE_LINEAR_ACCELERATION 0x00'00'00'0a
#define ASENSOR_TYPE_ORIENTATION         0x00'00'00'03

namespace
{
ALooper*                                                            looper;
ASensorManager*                                                     sensorManager;
ASensorEventQueue*                                                  sensorEventQueue;
sf::base::EnumArray<sf::Sensor::Type, sf::Vec3f, sf::Sensor::Count> sensorData;
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
void SensorImpl::initialize()
{
    // Get the looper associated with this thread
    looper = ALooper_forThread();

// Get the unique sensor manager
#if ANDROID_API >= 26 || __ANDROID_API__ >= 26
    sensorManager = ASensorManager_getInstanceForPackage(nullptr);
#else
    sensorManager = ASensorManager_getInstance();
#endif

    // Create the sensor events queue and attach it to the looper
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 1, &processSensorEvents, nullptr);
}


////////////////////////////////////////////////////////////
void SensorImpl::cleanup()
{
    // Detach the sensor events queue from the looper and destroy it
    ASensorManager_destroyEventQueue(sensorManager, sensorEventQueue);
}


////////////////////////////////////////////////////////////
bool SensorImpl::isAvailable(Sensor::Type sensor)
{
    const ASensor* available = getDefaultSensor(sensor);

    return available != nullptr;
}


////////////////////////////////////////////////////////////
bool SensorImpl::open(Sensor::Type sensor)
{
    // Get the default sensor matching the type
    m_sensor = getDefaultSensor(sensor);

    // Sensor not available, stop here
    if (!m_sensor)
        return false;

    // Set the event rate (not to consume too much battery)
    ASensorEventQueue_setEventRate(sensorEventQueue, m_sensor, ASensor_getMinDelay(m_sensor));

    // Save the type of the sensor
    m_type = sensor;

    return true;
}


////////////////////////////////////////////////////////////
void SensorImpl::close()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
Vec3f SensorImpl::update() const
{
    // Update our sensor data list
    while (ALooper_pollOnce(0, nullptr, nullptr, nullptr) >= 0)
        ;

    return sensorData[m_type];
}


////////////////////////////////////////////////////////////
void SensorImpl::setEnabled(bool enabled)
{
    if (enabled)
        ASensorEventQueue_enableSensor(sensorEventQueue, m_sensor);
    else
        ASensorEventQueue_disableSensor(sensorEventQueue, m_sensor);
}


////////////////////////////////////////////////////////////
const ASensor* SensorImpl::getDefaultSensor(Sensor::Type sensor)
{
    // Find the Android sensor type
    static constexpr base::EnumArray<Sensor::Type, int, Sensor::Count> types =
        {ASENSOR_TYPE_ACCELEROMETER,
         ASENSOR_TYPE_GYROSCOPE,
         ASENSOR_TYPE_MAGNETIC_FIELD,
         ASENSOR_TYPE_GRAVITY,
         ASENSOR_TYPE_LINEAR_ACCELERATION,
         ASENSOR_TYPE_ORIENTATION};

    const int type = types[sensor];

    // Retrieve the default sensor matching this type
    return ASensorManager_getDefaultSensor(sensorManager, type);
}


////////////////////////////////////////////////////////////
int SensorImpl::processSensorEvents(int /* fd */, int /* events */, void* /* sensorData */)
{
    ASensorEvent event;

    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        base::Optional<Sensor::Type> type;
        Vec3f                        data;

        switch (event.type)
        {
            case ASENSOR_TYPE_ACCELEROMETER:
                type   = Sensor::Type::Accelerometer;
                data.x = event.acceleration.x;
                data.y = event.acceleration.y;
                data.z = event.acceleration.z;
                break;

            case ASENSOR_TYPE_GYROSCOPE:
                type   = Sensor::Type::Gyroscope;
                data.x = event.vector.x;
                data.y = event.vector.y;
                data.z = event.vector.z;
                break;

            case ASENSOR_TYPE_MAGNETIC_FIELD:
                type   = Sensor::Type::Magnetometer;
                data.x = event.magnetic.x;
                data.y = event.magnetic.y;
                data.z = event.magnetic.z;
                break;

            case ASENSOR_TYPE_GRAVITY:
                type   = Sensor::Type::Gravity;
                data.x = event.vector.x;
                data.y = event.vector.y;
                data.z = event.vector.z;
                break;

            case ASENSOR_TYPE_LINEAR_ACCELERATION:
                type   = Sensor::Type::UserAcceleration;
                data.x = event.acceleration.x;
                data.y = event.acceleration.y;
                data.z = event.acceleration.z;
                break;

            case ASENSOR_TYPE_ORIENTATION:
                type   = Sensor::Type::Orientation;
                data.x = event.vector.x;
                data.y = event.vector.y;
                data.z = event.vector.z;
                break;
        }

        // An unknown sensor event has been detected, we don't know how to process it
        if (!type)
            continue;

        sensorData[*type] = data;
    }

    return 1;
}

} // namespace sf::priv
