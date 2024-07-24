#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

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
