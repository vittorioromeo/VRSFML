#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Shader.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] MainShaders
{
    ////////////////////////////////////////////////////////////
    // Shader with hue support and bubble effects
    sf::Shader shader;

    sf::Shader::UniformLocation suBackgroundTexture;
    sf::Shader::UniformLocation suTime;
    sf::Shader::UniformLocation suResolution;
    sf::Shader::UniformLocation suBackgroundOrigin;
    sf::Shader::UniformLocation suBubbleEffect;

    sf::Shader::UniformLocation suIridescenceStrength;
    sf::Shader::UniformLocation suEdgeFactorMin;
    sf::Shader::UniformLocation suEdgeFactorMax;
    sf::Shader::UniformLocation suEdgeFactorStrength;
    sf::Shader::UniformLocation suDistorsionStrength;

    sf::Shader::UniformLocation suSubTexOrigin;
    sf::Shader::UniformLocation suSubTexSize;

    sf::Shader::UniformLocation suBubbleLightness;
    sf::Shader::UniformLocation suLensDistortion;

    sf::Shader::UniformLocation suRimShineStrength;
    sf::Shader::UniformLocation suRimShineFallRate;
    sf::Shader::UniformLocation suRimShineTimeRate;
    sf::Shader::UniformLocation suRimShineArc;

    ////////////////////////////////////////////////////////////
    // Shader with post-processing effects
    sf::Shader shaderPostProcess;

    sf::Shader::UniformLocation suPPVibrance;
    sf::Shader::UniformLocation suPPSaturation;
    sf::Shader::UniformLocation suPPLightness;
    sf::Shader::UniformLocation suPPSharpness;
    sf::Shader::UniformLocation suPPBlur;

    ////////////////////////////////////////////////////////////
    // Shader for fluffy cat cloud rendering
    sf::Shader shaderClouds;

    sf::Shader::UniformLocation suCloudTime;
    sf::Shader::UniformLocation suCloudResolution;

    ////////////////////////////////////////////////////////////
    // Shader for hexed cat phasing/distortion
    sf::Shader shaderHexed;

    sf::Shader::UniformLocation suHexedTime;
    sf::Shader::UniformLocation suHexedSeed;
    sf::Shader::UniformLocation suHexedDistortionStrength;
    sf::Shader::UniformLocation suHexedShimmerStrength;

    ////////////////////////////////////////////////////////////
    // Shader for activated shrine background distortion
    sf::Shader shaderShrineBackground;

    sf::Shader::UniformLocation suShrineBgTime;
    sf::Shader::UniformLocation suShrineBgViewOrigin;
    sf::Shader::UniformLocation suShrineBgCenter;
    sf::Shader::UniformLocation suShrineBgRange;
    sf::Shader::UniformLocation suShrineBgTintR;
    sf::Shader::UniformLocation suShrineBgTintG;
    sf::Shader::UniformLocation suShrineBgTintB;
    sf::Shader::UniformLocation suShrineBgTintA;
    sf::Shader::UniformLocation suShrineBgDistortionStrength;
    sf::Shader::UniformLocation suShrineBgTintStrength;
    sf::Shader::UniformLocation suShrineBgEffectStrength;
};
