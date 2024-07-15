////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/SizeT.hpp>
#include <SFML/Base/Traits/IsEnum.hpp>


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Fixed-size array container indexed by an enumeration
///
////////////////////////////////////////////////////////////
template <typename Enum, typename Value, SizeT Count>
struct EnumArray
{
    static_assert(SFML_BASE_IS_ENUM(Enum));

    ////////////////////////////////////////////////////////////
    /// \brief Returns a reference to the element associated to specified \a key
    ///
    /// No bounds checking is performed in release builds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Value& operator[](Enum key)
    {
        const auto index = static_cast<SizeT>(key);
        SFML_BASE_ASSERT(index < Count && "Index is out of bounds");
        return data[index];
    }

    ////////////////////////////////////////////////////////////
    /// \brief Returns a reference to the element associated to specified \a key
    ///
    /// No bounds checking is performed in release builds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const Value& operator[](Enum key) const
    {
        const auto index = static_cast<SizeT>(key);
        SFML_BASE_ASSERT(index < Count && "Index is out of bounds");
        return data[index];
    }

    [[gnu::always_inline]] constexpr void fill(Value key)
    {
        for (Value& value : data)
            value = key;
    }

    Value data[Count];
};

} // namespace sf::base
