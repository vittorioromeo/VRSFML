#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

// Repeating isEventSubtype<T> allows for cleaner compiler errors.
// It is not strictly necessary but it's useful nonetheless.
// It works by ensuring that the code within the conditional is
// only compiled when the condition is met. Otherwise you get
// a static_assert failure in addition to the compiler failing
// to compile the code within the compiletime conditional when
// an incorrect template parameter is provided.

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Event.hpp> // NOLINT(misc-header-include-cycle)


namespace sf
{
////////////////////////////////////////////////////////////
template <typename Visitor>
decltype(auto) Event::visit(Visitor&& visitor) const
{
    return m_data.linear_visit(static_cast<Visitor&&>(visitor));
}


////////////////////////////////////////////////////////////
template <typename... Handlers>
decltype(auto) Event::match(Handlers&&... handlers) const
{
    return m_data.linear_match(static_cast<Handlers&&>(handlers)...);
}

} // namespace sf
