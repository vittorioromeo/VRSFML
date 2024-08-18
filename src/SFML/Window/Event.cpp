#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp"


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
