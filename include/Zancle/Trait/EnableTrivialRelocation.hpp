#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Opt a class into trivial relocatability when the given predicate is true.
///
////////////////////////////////////////////////////////////
#define ZA_ENABLE_TRIVIAL_RELOCATION_IF(...)    \
    enum : bool                                 \
    {                                           \
        enableTrivialRelocation = (__VA_ARGS__) \
    }


////////////////////////////////////////////////////////////
/// \brief Opt a class into trivial relocatability unconditionally
///
/// Shorthand for `ZA_ENABLE_TRIVIAL_RELOCATION_IF(true)`. Place inside
/// the class body:
///
/// \code
/// class Foo
/// {
/// public:
///     ZA_ENABLE_TRIVIAL_RELOCATION;
/// };
/// \endcode
///
////////////////////////////////////////////////////////////
#define ZA_ENABLE_TRIVIAL_RELOCATION ZA_ENABLE_TRIVIAL_RELOCATION_IF(true)
