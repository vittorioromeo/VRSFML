#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace zb
{
class String;
} // namespace zb

using DWORD = unsigned long;


namespace za::priv
{
////////////////////////////////////////////////////////////
zb::String getErrorString(DWORD error);

////////////////////////////////////////////////////////////
void setWindowBorderless(void* hwnd, unsigned int width, unsigned int height);

} // namespace za::priv
