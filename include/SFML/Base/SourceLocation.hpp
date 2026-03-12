#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#if !__has_builtin(__builtin_source_location)
    #error "Compiler does not support __builtin_source_location, which is required by sf::base::SourceLocation"
#endif


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/AssertAndAssume.hpp"


////////////////////////////////////////////////////////////
// NOLINTBEGIN

namespace std
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct source_location
{
    struct __impl
    {
        const char*  _M_file_name;
        const char*  _M_function_name;
        unsigned int _M_line;
        unsigned int _M_column;
    };
};

} // namespace std

// NOLINTEND
////////////////////////////////////////////////////////////


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SourceLocation
{
    using BuiltinTypePtr = decltype(__builtin_source_location());
    BuiltinTypePtr ptr   = nullptr;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static consteval SourceLocation current(
        BuiltinTypePtr ptr = __builtin_source_location()) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return {ptr};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr unsigned int line() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_line;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr unsigned int column() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_column;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const char* fileName() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_file_name;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const char* functionName() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_function_name;
    }
};

} // namespace sf::base
