#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
class String;
} // namespace sf::base

using DWORD = unsigned long;


namespace sf::priv
{
////////////////////////////////////////////////////////////
base::String getErrorString(DWORD error);

////////////////////////////////////////////////////////////
void setWindowBorderless(void* hwnd, unsigned int width, unsigned int height);

} // namespace sf::priv
