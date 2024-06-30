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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Event.hpp>


namespace
{
////////////////////////////////////////////////////////////
template <typename>
constexpr bool isEventSubtype = false;

#define SFML_PRIV_EVENT_X_IS_EVENT_SUBTYPE(x) \
    template <>                               \
    constexpr bool isEventSubtype<x> = true

SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_IS_EVENT_SUBTYPE, SFML_PRIV_EVENT_X_SEMICOLON);

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
template <typename TEventSubtype>
Event::Event(const TEventSubtype& eventSubtype)
{
    static_assert(isEventSubtype<TEventSubtype>, "TEventSubtype must be a subtype of sf::Event");
    if constexpr (isEventSubtype<TEventSubtype>)
        m_data = eventSubtype;
}


////////////////////////////////////////////////////////////
template <typename TEventSubtype>
bool Event::is() const
{
    static_assert(isEventSubtype<TEventSubtype>, "TEventSubtype must be a subtype of sf::Event");
    if constexpr (isEventSubtype<TEventSubtype>)
        return m_data.is<TEventSubtype>();
}


////////////////////////////////////////////////////////////
template <typename TEventSubtype>
const TEventSubtype* Event::getIf() const
{
    static_assert(isEventSubtype<TEventSubtype>, "TEventSubtype must be a subtype of sf::Event");
    if constexpr (isEventSubtype<TEventSubtype>)
        return m_data.is<TEventSubtype>() ? &m_data.as<TEventSubtype>() : nullptr;
}

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////

template class SFML_PRIV_EVENT_VARIANT_TYPE;

#define SFML_PRIV_EVENT_X_TEMPLATE_CTOR(x)  template sf::Event::Event(const x&);
#define SFML_PRIV_EVENT_X_TEMPLATE_IS(x)    template bool sf::Event::is<x>() const;
#define SFML_PRIV_EVENT_X_TEMPLATE_GETIF(x) template const x* sf::Event::getIf<x>() const;

SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_CTOR, SFML_PRIV_EVENT_X_SEMICOLON);
SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_IS, SFML_PRIV_EVENT_X_SEMICOLON);
SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_GETIF, SFML_PRIV_EVENT_X_SEMICOLON);
