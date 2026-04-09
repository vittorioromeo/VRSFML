#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class RenderTarget;
class Shader;
class Texture;
class WindowContext;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Explicit owner of the global graphics state for VRSFML
///
/// `sf::GraphicsContext` is a process-wide singleton that owns
/// the OpenGL ES function loader, the shared default shader,
/// the built-in 1x1 white texture, and various per-thread GL
/// context bookkeeping.
///
/// One `GraphicsContext` must exist before any other graphics
/// resource (`sf::RenderWindow`, `sf::RenderTexture`,
/// `sf::Texture`, ...) is created. The recommended pattern is
/// to instantiate it once at the very top of `main`, hold it
/// in a local variable, and pass it (or rely on its existence)
/// throughout the program. When the local variable goes out
/// of scope, every graphics resource is shut down deterministically.
///
/// VRSFML deliberately avoids hidden global state, which is
/// why this class is a first-class object instead of a magical
/// auto-init.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API GraphicsContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create the process-wide graphics context
    ///
    /// Initializes the underlying window context, loads OpenGL
    /// ES function pointers, and registers the built-in shader
    /// and the built-in 1x1 white texture. Only one
    /// `GraphicsContext` may be alive at a time.
    ///
    /// \return `GraphicsContext` on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<GraphicsContext> create();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GraphicsContext(base::PassKey<GraphicsContext>&&, WindowContext&& windowContext);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~GraphicsContext();

    // Deleted copy operations
    GraphicsContext(const GraphicsContext&)            = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    /// Move assignment is intentionally deleted: at most one
    /// `GraphicsContext` may exist at a time, and reseating an
    /// already-installed context is not supported.
    ///
    ////////////////////////////////////////////////////////////
    GraphicsContext(GraphicsContext&& rhs) noexcept;

    // Deleted move assignment
    GraphicsContext& operator=(GraphicsContext&& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Returns the built-in shader
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Shader& getBuiltInShader();

    ////////////////////////////////////////////////////////////
    /// \brief Returns the built-in 1x1 white texture
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Texture& getBuiltInWhiteDotTexture();

    ////////////////////////////////////////////////////////////
    /// \brief Returns `true` if a `GraphicsContext` is installed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isInstalled();

    ////////////////////////////////////////////////////////////
    /// \brief Get the currently active context's ID
    ///
    /// The context ID is used to identify contexts when
    /// managing unshareable OpenGL resources.
    ///
    /// \return The active context's ID or 0 if no context is currently active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getActiveThreadLocalGlContextId();

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the calling thread has an active OpenGL context
    ///
    /// \return `true` if a context is currently bound on this thread, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool hasActiveThreadLocalGlContext();

    ////////////////////////////////////////////////////////////
    /// \brief Track an unshared framebuffer object so it can be cleaned up later
    ///
    /// FBOs are not shared between OpenGL contexts. The
    /// graphics context tracks each FBO together with the GL
    /// context that created it so that it can be released when
    /// the owning context goes away.
    ///
    /// \param glContextId   ID of the GL context that owns the FBO
    /// \param frameBufferId OpenGL name of the FBO
    ///
    ////////////////////////////////////////////////////////////
    static void registerUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId);

    ////////////////////////////////////////////////////////////
    /// \brief Stop tracking a previously registered framebuffer object
    ///
    /// \param glContextId   ID of the GL context that owns the FBO
    /// \param frameBufferId OpenGL name of the FBO
    ///
    ////////////////////////////////////////////////////////////
    static void unregisterUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId);

    ////////////////////////////////////////////////////////////
    /// \brief Track an unshared VAO so it can be cleaned up later
    ///
    /// VAOs are not shared between OpenGL contexts; see
    /// `registerUnsharedFrameBuffer` for the rationale.
    ///
    /// \param glContextId ID of the GL context that owns the VAO
    /// \param vaoId       OpenGL name of the VAO
    ///
    ////////////////////////////////////////////////////////////
    static void registerUnsharedVAO(unsigned int glContextId, unsigned int vaoId);

    ////////////////////////////////////////////////////////////
    /// \brief Stop tracking a previously registered VAO
    ///
    /// \param glContextId ID of the GL context that owns the VAO
    /// \param vaoId       OpenGL name of the VAO
    ///
    ////////////////////////////////////////////////////////////
    static void unregisterUnsharedVAO(unsigned int glContextId, unsigned int vaoId);

private:
    friend Shader;
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief Returns the built-in shader (private `static` version)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Shader& getInstalledBuiltInShader();

    ////////////////////////////////////////////////////////////
    /// \brief Returns the built-in 1x1 white texture (private `static` version)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Texture& getInstalledBuiltInWhiteDotTexture();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::GraphicsContext
/// \ingroup graphics
///
/// `sf::GraphicsContext` is the explicit, RAII-managed
/// container for VRSFML's graphics global state. It owns the
/// shared OpenGL ES function loader, the built-in default
/// shader, and the built-in 1x1 white texture (used by every
/// non-textured draw call).
///
/// VRSFML refuses to maintain hidden global state, so the
/// `GraphicsContext` must be created **explicitly**, before
/// any `sf::RenderWindow`, `sf::RenderTexture`, `sf::Texture`,
/// `sf::Shader`, or `sf::Font` is constructed. The recommended
/// pattern is to create one local instance at the top of
/// `main` and let it live until the program exits.
///
/// \code
/// int main()
/// {
///     auto gfx = sf::GraphicsContext::create().value();
///
///     auto window = sf::RenderWindow::create({/* ... */}).value();
///     // ... rest of the program ...
///
///     return 0;
/// } // 'gfx' is destroyed last, after every other graphics resource.
/// \endcode
///
/// Only one `GraphicsContext` may be alive at a time. The
/// class is move-only and move-assignment is deleted to make
/// the "install once" semantics explicit.
///
/// \see `sf::AudioContext`, `sf::WindowContext`,
///      `sf::RenderWindow`, `sf::RenderTexture`,
///      `sf::DefaultShader`
///
////////////////////////////////////////////////////////////
