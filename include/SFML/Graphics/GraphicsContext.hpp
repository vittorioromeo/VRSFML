#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/Window/WindowContext.hpp>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class GlContext;
class RenderTextureImplDefault;
class RenderTextureImplFBO;
} // namespace sf::priv

namespace sf
{
class Shader;
class Texture;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] GraphicsContext : public WindowContext
{
public:
    explicit GraphicsContext();
    ~GraphicsContext();

    [[nodiscard]] Shader&  getBuiltInTexturedShader();
    [[nodiscard]] Texture& getBuiltInWhiteDotTexture();

private:
    friend Shader;
    friend priv::RenderTextureImplDefault;
    friend priv::RenderTextureImplFBO;

    using WindowContext::createGlContext; // Needed by befriended render texture implementations

    [[nodiscard]] const char* getBuiltInTexturedShaderVertexSrc() const;
    [[nodiscard]] const char* getBuiltInTexturedShaderFragmentSrc() const;

    ////////////////////////////////////////////////////////////
    /// Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 512> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::GraphicsContext
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::GraphicsContext
///
////////////////////////////////////////////////////////////
