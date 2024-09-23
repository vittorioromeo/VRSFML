#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp" // NOLINT(misc-header-include-cycle)


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
