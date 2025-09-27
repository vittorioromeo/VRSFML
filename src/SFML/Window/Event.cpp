// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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

// Repeating isEventSubtype<T> allows for cleaner compiler errors.
// It is not strictly necessary but it's useful nonetheless.
// It works by ensuring that the code within the conditional is
// only compiled when the condition is met. Otherwise you get
// a static_assert failure in addition to the compiler failing
// to compile the code within the compiletime conditional when
// an incorrect template parameter is provided.

namespace sf
{
////////////////////////////////////////////////////////////
template <typename TEventSubtype>
Event::Event(const TEventSubtype& eventSubtype)
{
    static_assert(isEventSubtype<TEventSubtype>, "`TEventSubtype` must be a subtype of `sf::Event`");
    if constexpr (isEventSubtype<TEventSubtype>)
        m_data = eventSubtype;
}


////////////////////////////////////////////////////////////
template <typename TEventSubtype>
bool Event::is() const
{
    static_assert(isEventSubtype<TEventSubtype>, "`TEventSubtype` must be a subtype of `sf::Event`");
    if constexpr (isEventSubtype<TEventSubtype>)
        return m_data.is<TEventSubtype>();
}


////////////////////////////////////////////////////////////
template <typename TEventSubtype>
TEventSubtype* Event::getIf()
{
    static_assert(isEventSubtype<TEventSubtype>, "`TEventSubtype` must be a subtype of `sf::Event`");
    if constexpr (isEventSubtype<TEventSubtype>)
        return m_data.get_if<TEventSubtype>();
}


////////////////////////////////////////////////////////////
template <typename TEventSubtype>
const TEventSubtype* Event::getIf() const
{
    static_assert(isEventSubtype<TEventSubtype>, "`TEventSubtype` must be a subtype of `sf::Event`");
    if constexpr (isEventSubtype<TEventSubtype>)
        return m_data.get_if<TEventSubtype>();
}

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////

template class SFML_PRIV_EVENT_VARIANT_TYPE;

#define SFML_PRIV_EVENT_X_TEMPLATE_CTOR(x)        template sf::Event::Event(const x&);
#define SFML_PRIV_EVENT_X_TEMPLATE_IS(x)          template bool sf::Event::is<x>() const;
#define SFML_PRIV_EVENT_X_TEMPLATE_GETIF(x)       template x* sf::Event::getIf<x>();
#define SFML_PRIV_EVENT_X_TEMPLATE_GETIF_CONST(x) template const x* sf::Event::getIf<x>() const;

SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_CTOR, SFML_PRIV_EVENT_X_SEMICOLON);
SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_IS, SFML_PRIV_EVENT_X_SEMICOLON);
SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_GETIF, SFML_PRIV_EVENT_X_SEMICOLON);
SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_TEMPLATE_GETIF_CONST, SFML_PRIV_EVENT_X_SEMICOLON);
