#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
namespace std
{
////////////////////////////////////////////////////////////
enum class align_val_t : decltype(sizeof(0));

} // namespace std


////////////////////////////////////////////////////////////
void* operator new(decltype(sizeof(0)), std::align_val_t);
void  operator delete(void*, std::align_val_t) noexcept;
