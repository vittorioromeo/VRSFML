#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderStates.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
// We cannot use the default constructor here, because it accesses BlendAlpha, which is also global (and dynamically
// initialized). Initialization order of global objects in different translation units is not defined.
const RenderStates RenderStates::Default{
    .blendMode = BlendMode(BlendMode::Factor::SrcAlpha,
                           BlendMode::Factor::OneMinusSrcAlpha,
                           BlendMode::Equation::Add,
                           BlendMode::Factor::One,
                           BlendMode::Factor::OneMinusSrcAlpha,
                           BlendMode::Equation::Add)};

} // namespace sf
