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
const RenderStates RenderStates::Default(BlendMode(
    BlendMode::Factor::SrcAlpha,
    BlendMode::Factor::OneMinusSrcAlpha,
    BlendMode::Equation::Add,
    BlendMode::Factor::One,
    BlendMode::Factor::OneMinusSrcAlpha,
    BlendMode::Equation::Add));


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const Transform& theTransform) : transform(theTransform)
{
}


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const BlendMode& theBlendMode) : blendMode(theBlendMode)
{
}


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const StencilMode& theStencilMode) : stencilMode(theStencilMode)
{
}


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const Texture* theTexture) : texture(theTexture)
{
}


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const Shader* theShader) : shader(theShader)
{
}


////////////////////////////////////////////////////////////
RenderStates::RenderStates(const BlendMode&   theBlendMode,
                           const StencilMode& theStencilMode,
                           const Transform&   theTransform,
                           CoordinateType     theCoordinateType,
                           const Texture*     theTexture,
                           const Shader*      theShader) :
blendMode(theBlendMode),
stencilMode(theStencilMode),
transform(theTransform),
coordinateType(theCoordinateType),
texture(theTexture),
shader(theShader)
{
}

} // namespace sf
