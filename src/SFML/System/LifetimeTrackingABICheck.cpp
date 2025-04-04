#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


////////////////////////////////////////////////////////////
extern "C"
{
#ifdef SFML_ENABLE_LIFETIME_TRACKING
    void sfmlInternalAbiCheckLifetimeTrackingEnabled()
    {
    }
#else
    void sfmlInternalAbiCheckLifetimeTrackingDisabled()
    {
    }
#endif
}
