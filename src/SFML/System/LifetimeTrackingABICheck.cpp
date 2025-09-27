// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
