////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2017 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_NONCOPYABLE_HPP
#define SFML_NONCOPYABLE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility class that makes any derived
///        class non-copyable
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API NonCopyable
{
protected:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Because this class has a copy constructor, the compiler
    /// will not automatically generate the default constructor.
    /// That's why we must mark it as defaulted explicitly.
    ///
    ////////////////////////////////////////////////////////////
    NonCopyable() = default;
    
    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ///
    /// By declaring a protected destructor it's impossible to
    /// call delete on a pointer of sf::NonCopyable, thus
    /// preventing possible resource leaks.
    ///
    ////////////////////////////////////////////////////////////
    ~NonCopyable() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Disabled copy constructor
    ///
    /// By marking the copy constructor as deleted, the compiler
    /// will trigger an error if anyone outside tries to use it.
    ///
    ////////////////////////////////////////////////////////////
    NonCopyable(const NonCopyable&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Disabled copy assignment operator
    ///
    /// By marking the copy assignment operator as deleted, the
    /// compiler will trigger an error if anyone outside tries
    /// to use it.
    ///
    ////////////////////////////////////////////////////////////
    NonCopyable& operator =(const NonCopyable&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Default move constructor
    ///
    /// Because this class has all other special member
    /// functions user-defined, the compiler will not
    /// automatically generate the default move constructor.
    /// That's why we must mark it as defaulted explicitly.
    ///
    ////////////////////////////////////////////////////////////
    NonCopyable(NonCopyable&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Default move assignment operator
    ///
    /// Because this class has all other special member
    /// functions user-defined, the compiler will not
    /// automatically generate the default move assignment
    /// operator. That's why we must mark it as defaulted
    /// explicitly.
    ///
    ////////////////////////////////////////////////////////////
    NonCopyable& operator =(NonCopyable&&) = default;
};

} // namespace sf


#endif // SFML_NONCOPYABLE_HPP


////////////////////////////////////////////////////////////
/// \class sf::NonCopyable
/// \ingroup system
///
/// This class makes its instances non-copyable, by
/// explicitly deleting its copy constructor and its
/// assignment operator.
///
/// To create a non-copyable class, simply inherit from
/// sf::NonCopyable.
///
/// The type of inheritance (public or private) doesn't
/// matter, the copy constructor and assignment operator
/// are deleted in sf::NonCopyable so they will end up being
/// inaccessible in both cases. Thus you can use a shorter
/// syntax for inheriting from it (see below).
///
/// Usage example:
/// \code
/// class MyNonCopyableClass : sf::NonCopyable
/// {
///     ...
/// };
/// \endcode
///
/// Deciding whether the instances of a class can be copied
/// or not is a very important design choice. You are
/// strongly encouraged to think about it before writing a
/// class, and to use sf::NonCopyable when necessary to
/// prevent many potential future errors when using it. This
/// is alsoa very important indication to users of your
/// class.
///
////////////////////////////////////////////////////////////
