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
/// \brief Forward declaration of `std::source_location`
///
/// Required for `__builtin_source_location` to work without including
/// the heavy `<source_location>` standard header. The internal layout
/// must match the one expected by the compiler builtin.
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
/// \brief Lightweight replacement for `std::source_location`
///
/// Captures the file name, function name, line number, and column
/// at the call site through the `__builtin_source_location` compiler
/// builtin. Avoids the cost of including the `<source_location>`
/// standard header.
///
/// Use `SourceLocation::current()` as a default function argument to
/// capture the caller's location automatically.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SourceLocation
{
    using BuiltinTypePtr = decltype(__builtin_source_location());
    BuiltinTypePtr ptr   = nullptr;


    ////////////////////////////////////////////////////////////
    /// \brief Capture the current source location at the call site
    ///
    /// Intended to be used as a default argument so that the captured
    /// location refers to the caller, not to this function.
    ///
    /// \return A `SourceLocation` describing the call site
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static consteval SourceLocation current(
        BuiltinTypePtr ptr = __builtin_source_location()) noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return {ptr};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the line number associated with the source location
    ///
    /// \return Line number of the captured source location
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr unsigned int line() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_line;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the column number associated with the source location
    ///
    /// \return Column number of the captured source location
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr unsigned int column() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_column;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the file name associated with the source location
    ///
    /// \return Null-terminated string containing the file path
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const char* fileName() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_file_name;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the function name associated with the source location
    ///
    /// \return Null-terminated string containing the enclosing function name
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const char* functionName() const noexcept
    {
        SFML_BASE_ASSERT_AND_ASSUME(ptr != nullptr);
        return ptr->_M_function_name;
    }
};

} // namespace sf::base
