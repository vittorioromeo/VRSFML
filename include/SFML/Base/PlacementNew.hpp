#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr void* operator new(decltype(sizeof(int)), int, void* ptr) noexcept
{
    return ptr;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr void operator delete(void*, int, void*) noexcept
{
}


////////////////////////////////////////////////////////////
#define SFML_BASE_PLACEMENT_NEW(...) ::new (int{}, __VA_ARGS__)
