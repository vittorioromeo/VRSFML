#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/WindowEnums.hpp> // Prevent conflict with macro None from Xlib

#include <X11/Xlib.h>

#include <memory>
#include <string>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get the shared Display
///
/// \return Pointer to the shared display
///
////////////////////////////////////////////////////////////
std::shared_ptr<Display> openDisplay();

////////////////////////////////////////////////////////////
/// \brief Get the shared XIM context for the Display
///
/// \return XIM handle (a pointer) of the context
///
////////////////////////////////////////////////////////////
std::shared_ptr<_XIM> openXim();

////////////////////////////////////////////////////////////
/// \brief Get the atom with the specified name
///
/// \param name         Name of the atom
/// \param onlyIfExists Don't try to create the atom if it doesn't already exist
///
/// \return Atom if it exists or None (0) if it doesn't
///
////////////////////////////////////////////////////////////
Atom getAtom(const std::string& name, bool onlyIfExists = false);

} // namespace sf::priv
