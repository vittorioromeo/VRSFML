#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/BlendMode.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
// Commonly used blending modes
////////////////////////////////////////////////////////////
const BlendMode BlendAlpha(BlendMode::Factor::SrcAlpha,
                           BlendMode::Factor::OneMinusSrcAlpha,
                           BlendMode::Equation::Add,
                           BlendMode::Factor::One,
                           BlendMode::Factor::OneMinusSrcAlpha,
                           BlendMode::Equation::Add);

const BlendMode BlendAdd(BlendMode::Factor::SrcAlpha,
                         BlendMode::Factor::One,
                         BlendMode::Equation::Add,
                         BlendMode::Factor::One,
                         BlendMode::Factor::One,
                         BlendMode::Equation::Add);

const BlendMode BlendMultiply(BlendMode::Factor::DstColor, BlendMode::Factor::Zero, BlendMode::Equation::Add);
const BlendMode BlendMin(BlendMode::Factor::One, BlendMode::Factor::One, BlendMode::Equation::Min);
const BlendMode BlendMax(BlendMode::Factor::One, BlendMode::Factor::One, BlendMode::Equation::Max);
const BlendMode BlendNone(BlendMode::Factor::One, BlendMode::Factor::Zero, BlendMode::Equation::Add);


////////////////////////////////////////////////////////////
BlendMode::BlendMode(Factor sourceFactor, Factor destinationFactor, Equation blendEquation) :
colorSrcFactor(sourceFactor),
colorDstFactor(destinationFactor),
colorEquation(blendEquation),
alphaSrcFactor(sourceFactor),
alphaDstFactor(destinationFactor),
alphaEquation(blendEquation)
{
}


////////////////////////////////////////////////////////////
BlendMode::BlendMode(Factor   colorSourceFactor,
                     Factor   colorDestinationFactor,
                     Equation colorBlendEquation,
                     Factor   alphaSourceFactor,
                     Factor   alphaDestinationFactor,
                     Equation alphaBlendEquation) :
colorSrcFactor(colorSourceFactor),
colorDstFactor(colorDestinationFactor),
colorEquation(colorBlendEquation),
alphaSrcFactor(alphaSourceFactor),
alphaDstFactor(alphaDestinationFactor),
alphaEquation(alphaBlendEquation)
{
}

} // namespace sf
