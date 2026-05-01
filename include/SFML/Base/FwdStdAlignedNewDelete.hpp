#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Forward declarations of `std::align_val_t` and aligned `new`/`delete`
///
/// Allows code to use the aligned-allocation overloads of `operator new`
/// and `operator delete` without pulling in the heavy `<new>` header.
/// clangd is given the real header so its semantic analysis stays
/// consistent with the rest of the standard library.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#ifdef __CLANGD__
    #include <new> // IWYU pragma: export
#endif


////////////////////////////////////////////////////////////
namespace std
{
////////////////////////////////////////////////////////////
enum class align_val_t : decltype(sizeof(0));

} // namespace std


////////////////////////////////////////////////////////////
void* operator new(decltype(sizeof(0)), std::align_val_t);
void  operator delete(void*, std::align_val_t) noexcept;
