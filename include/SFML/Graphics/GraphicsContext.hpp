#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Window/WindowContext.hpp"

#include "SFML/Base/InPlacePImpl.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class GlContext;
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

    [[nodiscard]] Shader&  getBuiltInShader();
    [[nodiscard]] Texture& getBuiltInWhiteDotTexture();

private:
    friend Shader;
    friend priv::RenderTextureImplFBO;

    using WindowContext::createGlContext; // Needed by befriended render texture implementations

    [[nodiscard]] const char* getBuiltInShaderVertexSrc() const;
    [[nodiscard]] const char* getBuiltInShaderFragmentSrc() const;

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
