#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
#ifdef __UINTPTR_TYPE__
using UIntPtrT = __UINTPTR_TYPE__;
#else
    #error "Could not determine a uintptr equivalent type (GCC/Clang)."
#endif

} // namespace sf::base
