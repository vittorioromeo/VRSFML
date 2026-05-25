#pragma once

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <>
struct StringMaker<sf::base::StringView>
{
    static sf::base::String convert(const sf::base::StringView& sv)
    {
        return sf::base::String{sv.data(), sv.size()};
    }
};

} // namespace tst
