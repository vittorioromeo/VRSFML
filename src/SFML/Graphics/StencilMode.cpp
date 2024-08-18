#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/StencilMode.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
StencilValue::StencilValue(int theValue) : value(static_cast<unsigned int>(theValue))
{
}


////////////////////////////////////////////////////////////
StencilValue::StencilValue(unsigned int theValue) : value(theValue)
{
}


////////////////////////////////////////////////////////////
bool operator==(const StencilMode& left, const StencilMode& right)
{
    return left.stencilUpdateOperation == right.stencilUpdateOperation &&
           left.stencilComparison == right.stencilComparison &&
           left.stencilReference.value == right.stencilReference.value &&
           left.stencilMask.value == right.stencilMask.value && left.stencilOnly == right.stencilOnly;
}


////////////////////////////////////////////////////////////
bool operator!=(const StencilMode& left, const StencilMode& right)
{
    return !(left == right);
}

} // namespace sf
