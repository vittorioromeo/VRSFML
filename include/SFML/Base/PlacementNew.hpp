#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Custom placement-new overload that does not require including `<new>`
///
/// The extra `int` parameter disambiguates this overload from the
/// standard placement-new declared in `<new>`, so this header can be
/// included without dragging in the entire `<new>` standard header.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr void* operator new(decltype(sizeof(int)), int, void* ptr) noexcept
{
    return ptr;
}


////////////////////////////////////////////////////////////
/// \brief Matching no-op placement-delete (required by the language for completeness)
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr void operator delete(void*, int, void*) noexcept
{
}


////////////////////////////////////////////////////////////
/// \brief Convenience macro: `SFML_BASE_PLACEMENT_NEW(addr) T(args...)`
///
/// Equivalent to `::new (addr) T(args...)` but uses the placement-new
/// overload above to avoid `<new>`.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_PLACEMENT_NEW(...) ::new (int{}, __VA_ARGS__)
