#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class String;
} // namespace za

using DWORD = unsigned long;


namespace za::priv
{
////////////////////////////////////////////////////////////
za::String getErrorString(DWORD error);

////////////////////////////////////////////////////////////
void setWindowBorderless(void* hwnd, unsigned int width, unsigned int height);

} // namespace za::priv
