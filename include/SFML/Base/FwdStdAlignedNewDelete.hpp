#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
namespace std
{
////////////////////////////////////////////////////////////
enum class align_val_t : decltype(sizeof(0));

} // namespace std


////////////////////////////////////////////////////////////
void* operator new(decltype(sizeof(0)), std::align_val_t);
void  operator delete(void*, std::align_val_t) noexcept;
