#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Shader.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] MainShaders
{
    ////////////////////////////////////////////////////////////
    // Shader with hue support and bubble effects
    za::Shader shader;

    za::Shader::UniformLocation suBackgroundTexture;
    za::Shader::UniformLocation suTime;
    za::Shader::UniformLocation suResolution;
    za::Shader::UniformLocation suBackgroundOrigin;
    za::Shader::UniformLocation suBubbleEffect;

    za::Shader::UniformLocation suIridescenceStrength;
    za::Shader::UniformLocation suEdgeFactorMin;
    za::Shader::UniformLocation suEdgeFactorMax;
    za::Shader::UniformLocation suEdgeFactorStrength;
    za::Shader::UniformLocation suDistorsionStrength;

    za::Shader::UniformLocation suSubTexOrigin;
    za::Shader::UniformLocation suSubTexSize;

    za::Shader::UniformLocation suBubbleLightness;
    za::Shader::UniformLocation suLensDistortion;

    za::Shader::UniformLocation suRimShineStrength;
    za::Shader::UniformLocation suRimShineFallRate;
    za::Shader::UniformLocation suRimShineTimeRate;
    za::Shader::UniformLocation suRimShineArc;

    ////////////////////////////////////////////////////////////
    // Shader with post-processing effects
    za::Shader shaderPostProcess;

    za::Shader::UniformLocation suPPVibrance;
    za::Shader::UniformLocation suPPSaturation;
    za::Shader::UniformLocation suPPLightness;
    za::Shader::UniformLocation suPPSharpness;
    za::Shader::UniformLocation suPPBlur;

    ////////////////////////////////////////////////////////////
    // Shader for fluffy cat cloud rendering
    za::Shader shaderClouds;

    za::Shader::UniformLocation suCloudTime;
    za::Shader::UniformLocation suCloudResolution;

    ////////////////////////////////////////////////////////////
    // Shader for hexed cat phasing/distortion
    za::Shader shaderHexed;

    za::Shader::UniformLocation suHexedTime;
    za::Shader::UniformLocation suHexedSeed;
    za::Shader::UniformLocation suHexedDistortionStrength;
    za::Shader::UniformLocation suHexedShimmerStrength;

    ////////////////////////////////////////////////////////////
    // Shader for activated shrine background distortion
    za::Shader shaderShrineBackground;

    za::Shader::UniformLocation suShrineBgTime;
    za::Shader::UniformLocation suShrineBgViewOrigin;
    za::Shader::UniformLocation suShrineBgCenter;
    za::Shader::UniformLocation suShrineBgRange;
    za::Shader::UniformLocation suShrineBgTintR;
    za::Shader::UniformLocation suShrineBgTintG;
    za::Shader::UniformLocation suShrineBgTintB;
    za::Shader::UniformLocation suShrineBgTintA;
    za::Shader::UniformLocation suShrineBgDistortionStrength;
    za::Shader::UniformLocation suShrineBgTintStrength;
    za::Shader::UniformLocation suShrineBgEffectStrength;
};
