#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureSaver.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"
#include "SFML/Window/Window.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/TrivialVector.hpp"

#include <atomic>
#include <utility>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace TextureImpl
{
// Thread-safe unique identifier generator, is used for states cache (see RenderTarget)
constinit std::atomic<unsigned int> nextUniqueId{1u}; // start at 1, zero is "no texture"

[[nodiscard, gnu::always_inline, gnu::flatten]] inline unsigned int getUniqueId() noexcept
{
    return nextUniqueId.fetch_add(1u, std::memory_order_relaxed);
}

} // namespace TextureImpl
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
Texture::Texture(base::PassKey<Texture>&&, Vector2u size, unsigned int texture, bool sRgb) :
m_size(size),
m_texture(texture),
m_sRgb(sRgb),
m_cacheId(TextureImpl::getUniqueId())
{
}


////////////////////////////////////////////////////////////
Texture::Texture(const Texture& rhs) :
m_isSmooth(rhs.m_isSmooth),
m_sRgb(rhs.m_sRgb),
m_isRepeated(rhs.m_isRepeated),
m_cacheId(TextureImpl::getUniqueId())
{
    base::Optional texture = create(rhs.getSize(), {.sRgb = rhs.isSrgb(), .smooth = rhs.isSmooth()});

    if (!texture.hasValue())
    {
        priv::err() << "Failed to copy texture, failed to create new texture";
        return;
    }

    *this = SFML_BASE_MOVE(*texture);

    if (!update(rhs))
        priv::err() << "Failed to copy texture, failed to update from new texture";
}


////////////////////////////////////////////////////////////
Texture::~Texture()
{
    // Destroy the OpenGL texture
    if (!m_texture)
        return;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    const GLuint texture = m_texture;
    glCheck(glDeleteTextures(1, &texture));
}


////////////////////////////////////////////////////////////
Texture::Texture(Texture&& right) noexcept :
m_size(base::exchange(right.m_size, {})),
m_texture(base::exchange(right.m_texture, 0u)),
m_isSmooth(base::exchange(right.m_isSmooth, false)),
m_sRgb(base::exchange(right.m_sRgb, false)),
m_isRepeated(base::exchange(right.m_isRepeated, false)),
m_fboAttachment(base::exchange(right.m_fboAttachment, false)),
m_hasMipmap(base::exchange(right.m_hasMipmap, false)),
m_cacheId(base::exchange(right.m_cacheId, 0u))
{
}


////////////////////////////////////////////////////////////
Texture& Texture::operator=(Texture&& right) noexcept
{
    // Catch self-moving.
    if (&right == this)
    {
        return *this;
    }

    // Destroy the OpenGL texture
    if (m_texture)
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

        const GLuint texture = m_texture;
        glCheck(glDeleteTextures(1, &texture));
    }

    // Move old to new.
    m_size          = base::exchange(right.m_size, {});
    m_texture       = base::exchange(right.m_texture, 0u);
    m_isSmooth      = base::exchange(right.m_isSmooth, false);
    m_sRgb          = base::exchange(right.m_sRgb, false);
    m_isRepeated    = base::exchange(right.m_isRepeated, false);
    m_fboAttachment = base::exchange(right.m_fboAttachment, false);
    m_hasMipmap     = base::exchange(right.m_hasMipmap, false);
    m_cacheId       = base::exchange(right.m_cacheId, 0u);

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::create(Vector2u size, const TextureCreateSettings& settings)
{
    base::Optional<Texture> result; // Use a single local variable for NRVO

    // Check if texture parameters are valid before creating it
    if ((size.x == 0) || (size.y == 0))
    {
        priv::err() << "Failed to create texture, invalid size (" << size.x << "x" << size.y << ")";
        return result; // Empty optional
    }

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Check the maximum texture size
    const unsigned int maxSize = getMaximumSize();
    if ((size.x > maxSize) || (size.y > maxSize))
    {
        priv::err() << "Failed to create texture, its internal size is too high "
                    << "(" << size.x << "x" << size.y << ", "
                    << "maximum is " << maxSize << "x" << maxSize << ")";

        return result; // Empty optional
    }

    // Create the OpenGL texture
    GLuint glTexture = 0;
    glCheck(glGenTextures(1, &glTexture));
    SFML_BASE_ASSERT(glTexture);

    // All the validity checks passed, we can store the new texture settings
    result.emplace(base::PassKey<Texture>{}, size, glTexture, settings.sRgb);
    Texture& texture = *result;

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    const GLint textureWrapParam = GL_CLAMP_TO_EDGE;

    // Initialize the texture
    glCheck(glBindTexture(GL_TEXTURE_2D, texture.m_texture));
    glCheck(glTexImage2D(GL_TEXTURE_2D,
                         0,
                         (texture.m_sRgb ? GL_SRGB8_ALPHA8 : GL_RGBA),
                         static_cast<GLsizei>(texture.m_size.x),
                         static_cast<GLsizei>(texture.m_size.y),
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         nullptr));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapParam));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapParam));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    texture.m_cacheId = TextureImpl::getUniqueId();

    texture.m_hasMipmap = false;

    result->setSmooth(settings.smooth);

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromFile(const Path& filename, const TextureLoadSettings& settings)
{
    if (const base::Optional image = sf::Image::loadFromFile(filename))
        return loadFromImage(*image, settings);

    priv::err() << "Failed to load texture from file";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromMemory(const void* data, base::SizeT size, const TextureLoadSettings& settings)
{
    if (const base::Optional image = sf::Image::loadFromMemory(data, size))
        return loadFromImage(*image, settings);

    priv::err() << "Failed to load texture from memory";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromStream(InputStream& stream, const TextureLoadSettings& settings)
{
    if (const base::Optional image = sf::Image::loadFromStream(stream))
        return loadFromImage(*image, settings);

    priv::err() << "Failed to load texture from stream";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromImage(const Image& image, const TextureLoadSettings& settings)
{
    base::Optional<Texture> result; // Use a single local variable for NRVO

    // Retrieve the image size
    const auto size = image.getSize().toVector2i();

    // Load the entire image if the source area is either empty or contains the whole image
    if (settings.area.size.x == 0 || (settings.area.size.y == 0) ||
        ((settings.area.position.x <= 0) && (settings.area.position.y <= 0) && (settings.area.size.x >= size.x) &&
         (settings.area.size.y >= size.y)))
    {
        // Load the entire image
        if ((result = sf::Texture::create(image.getSize(), {.sRgb = settings.sRgb, .smooth = settings.smooth})))
        {
            result->update(image);
            return result;
        }

        // Error message generated in called function.
        return result; // Empty optional
    }

    // Load a sub-area of the image

    // Adjust the rectangle to the size of the image
    IntRect rectangle    = settings.area;
    rectangle.position.x = base::max(rectangle.position.x, 0);
    rectangle.position.y = base::max(rectangle.position.y, 0);
    rectangle.size.x     = base::min(rectangle.size.x, size.x - rectangle.position.x);
    rectangle.size.y     = base::min(rectangle.size.y, size.y - rectangle.position.y);

    // Create the texture and upload the pixels
    if ((result = sf::Texture::create(rectangle.size.toVector2u(), {.sRgb = settings.sRgb, .smooth = settings.smooth})))
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

        // Make sure that the current texture binding will be preserved
        const priv::TextureSaver save;

        // Copy the pixels to the texture, row by row
        const base::U8* pixels = image.getPixelsPtr() + 4 * (rectangle.position.x + (size.x * rectangle.position.y));
        glCheck(glBindTexture(GL_TEXTURE_2D, result->m_texture));
        for (int i = 0; i < rectangle.size.y; ++i)
        {
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, rectangle.size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
            pixels += 4 * size.x;
        }

        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        result->m_hasMipmap = false;

        // Force an OpenGL flush, so that the texture will appear updated
        // in all contexts immediately (solves problems in multi-threaded apps)
        glCheck(glFlush());
    }

    result->setSmooth(settings.smooth);

    // Error message generated in called function.
    return result;
}


////////////////////////////////////////////////////////////
Vector2u Texture::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
Image Texture::copyToImage() const
{
    // Easy case: empty texture
    SFML_BASE_ASSERT(m_texture && "Texture::copyToImage Cannot copy empty texture to image");

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    // Create an array of pixels
    base::TrivialVector<base::U8> pixels(m_size.x * m_size.y * 4);

    // OpenGL ES doesn't have the glGetTexImage function, the only way to read
    // from a texture is to bind it to a FBO and use glReadPixels
    GLuint frameBuffer = 0;
    glCheck(glGenFramebuffers(1, &frameBuffer));
    if (frameBuffer)
    {
        const auto previousFrameBuffer = priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING);

        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
        glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0));
        glCheck(glReadPixels(0,
                             0,
                             static_cast<GLsizei>(m_size.x),
                             static_cast<GLsizei>(m_size.y),
                             GL_RGBA,
                             GL_UNSIGNED_BYTE,
                             pixels.data()));
        glCheck(glDeleteFramebuffers(1, &frameBuffer));

        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFrameBuffer)));
    }

    auto result = sf::Image::create(m_size, pixels.data());
    SFML_BASE_ASSERT(result.hasValue());
    return SFML_BASE_MOVE(*result);
}


////////////////////////////////////////////////////////////
void Texture::update(const base::U8* pixels)
{
    // Update the whole texture
    update(pixels, m_size, {0, 0});
}


////////////////////////////////////////////////////////////
void Texture::update(const base::U8* pixels, Vector2u size, Vector2u dest)
{
    SFML_BASE_ASSERT(dest.x + size.x <= m_size.x && "Destination x coordinate is outside of texture");
    SFML_BASE_ASSERT(dest.y + size.y <= m_size.y && "Destination y coordinate is outside of texture");

    SFML_BASE_ASSERT(pixels != nullptr);

    SFML_BASE_ASSERT(m_texture);
    SFML_BASE_ASSERT(glCheck(glIsTexture(m_texture)));

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    // Copy pixels from the given array to the texture
    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexSubImage2D(GL_TEXTURE_2D,
                            0,
                            static_cast<GLint>(dest.x),
                            static_cast<GLint>(dest.y),
                            static_cast<GLsizei>(size.x),
                            static_cast<GLsizei>(size.y),
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            pixels));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
    m_hasMipmap = false;
    m_cacheId   = TextureImpl::getUniqueId();

    // Force an OpenGL flush, so that the texture data will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());
}


////////////////////////////////////////////////////////////
bool Texture::update(const Texture& texture, Vector2u dest)
{
    SFML_BASE_ASSERT(dest.x + texture.m_size.x <= m_size.x && "Destination x coordinate is outside of texture");
    SFML_BASE_ASSERT(dest.y + texture.m_size.y <= m_size.y && "Destination y coordinate is outside of texture");

    SFML_BASE_ASSERT(m_texture);
    SFML_BASE_ASSERT(glCheck(glIsTexture(m_texture)));

    SFML_BASE_ASSERT(texture.m_texture);
    SFML_BASE_ASSERT(glCheck(glIsTexture(texture.m_texture)));

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Save the current bindings so we can restore them after we are done
    const auto readFramebuffer = priv::getGLInteger(GL_READ_FRAMEBUFFER_BINDING);
    const auto drawFramebuffer = priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING);

    // Create the framebuffers
    GLuint sourceFrameBuffer = 0;
    GLuint destFrameBuffer   = 0;
    glCheck(glGenFramebuffers(1, &sourceFrameBuffer));
    glCheck(glGenFramebuffers(1, &destFrameBuffer));

    if (!sourceFrameBuffer || !destFrameBuffer)
    {
        priv::err() << "Cannot copy texture, failed to create a frame buffer object";
        return false;
    }

    // Link the source texture to the source frame buffer
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFrameBuffer));
    glCheck(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.m_texture, 0));

    // Link the destination texture to the destination frame buffer
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBuffer));
    glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0));

    // A final check, just to be sure...
    GLenum sourceStatus = 0;
    glCheck(sourceStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    GLenum destStatus = 0;
    glCheck(destStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    bool success = true;

    if ((sourceStatus == GL_FRAMEBUFFER_COMPLETE) && (destStatus == GL_FRAMEBUFFER_COMPLETE))
    {
        // Scissor testing affects framebuffer blits as well
        // Since we don't want scissor testing to interfere with our copying, we temporarily disable it for the blit if it is enabled
        const priv::ScissorDisableGuard scissorDisableGuard;

        // Blit the texture contents from the source to the destination texture
        priv::blitFramebuffer(/* invertYAxis */ false, texture.m_size, {0u, 0u}, dest);
    }
    else
    {
        priv::err() << "Cannot copy texture, failed to link texture to frame buffer";
        success = false;
    }

    // Restore previously bound framebuffers
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer)));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer)));

    // Delete the framebuffers
    glCheck(glDeleteFramebuffers(1, &sourceFrameBuffer));
    glCheck(glDeleteFramebuffers(1, &destFrameBuffer));

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    // Set the parameters of this texture
    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
    m_hasMipmap = false;
    m_cacheId   = TextureImpl::getUniqueId();

    // Force an OpenGL flush, so that the texture data will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());
    return success;
}


////////////////////////////////////////////////////////////
void Texture::update(const Image& image, Vector2u dest)
{
    update(image.getPixelsPtr(), image.getSize(), dest);
}


////////////////////////////////////////////////////////////
bool Texture::update(const Window& window, Vector2u dest)
{
    SFML_BASE_ASSERT(dest.x + window.getSize().x <= m_size.x && "Destination x coordinate is outside of texture");
    SFML_BASE_ASSERT(dest.y + window.getSize().y <= m_size.y && "Destination y coordinate is outside of texture");

    SFML_BASE_ASSERT(m_texture);
    SFML_BASE_ASSERT(glCheck(glIsTexture(m_texture)));

    if (!window.setActive(true))
    {
        priv::err() << "Failed to activate window in `Texture::update`";
        return false;
    }

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Save the current bindings so we can restore them after we are done
    const auto readFramebuffer = priv::getGLInteger(GL_READ_FRAMEBUFFER_BINDING);
    const auto drawFramebuffer = priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING);

    // Create the destination framebuffers
    GLuint destFrameBuffer = 0;
    glCheck(glGenFramebuffers(1, &destFrameBuffer));

    GLuint sourceFrameBuffer = 0; // default fbo

    if (!destFrameBuffer)
    {
        priv::err() << "Cannot copy texture, failed to create a frame buffer object";
        return false;
    }

    // Link the source texture to the source frame buffer
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFrameBuffer));

    // Link the destination texture to the destination frame buffer
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBuffer));
    glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0));

    // A final check, just to be sure...
    const GLenum sourceStatus = glCheck(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));
    const GLenum destStatus   = glCheck(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    if ((sourceStatus == GL_FRAMEBUFFER_COMPLETE) && (destStatus == GL_FRAMEBUFFER_COMPLETE))
    {
        // Scissor testing affects framebuffer blits as well
        // Since we don't want scissor testing to interfere with our copying, we temporarily disable it for the blit if it is enabled
        const priv::ScissorDisableGuard scissorDisableGuard;

        // TODO P1: avoid creating this texture multiple times
        auto tmpTexture = Texture::create(window.getSize(), {.sRgb = m_sRgb, .smooth = m_isSmooth});

        if (!tmpTexture.hasValue())
            priv::err() << "Failure to create intermediate texture in `copyFlippedFramebuffer`";
        else if (!priv::copyFlippedFramebuffer(tmpTexture->getNativeHandle(),
                                               window.getSize(),
                                               sourceFrameBuffer,
                                               destFrameBuffer,
                                               {0u, 0u},
                                               dest))
            priv::err() << "Error flipping render texture during FBO copy";
    }
    else
    {
        priv::err() << "Cannot copy texture, failed to link texture to frame buffer";
    }

    // Restore previously bound framebuffers
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer)));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer)));

    // Delete the framebuffers
    glCheck(glDeleteFramebuffers(1, &destFrameBuffer));

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    // Set the parameters of this texture
    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
    m_hasMipmap = false;

    m_cacheId = TextureImpl::getUniqueId();

    // Force an OpenGL flush, so that the texture will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());

    return true;
}


////////////////////////////////////////////////////////////
void Texture::setSmooth(bool smooth)
{
    SFML_BASE_ASSERT(m_texture);

    if (smooth == m_isSmooth)
        return;

    m_isSmooth = smooth;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));

    if (m_hasMipmap)
    {
        glCheck(glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER,
                                m_isSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR));
    }
    else
    {
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
    }
}


////////////////////////////////////////////////////////////
bool Texture::isSmooth() const
{
    return m_isSmooth;
}


////////////////////////////////////////////////////////////
bool Texture::isSrgb() const
{
    return m_sRgb;
}


////////////////////////////////////////////////////////////
void Texture::setRepeated(bool repeated)
{
    SFML_BASE_ASSERT(m_texture);

    if (repeated == m_isRepeated)
        return;

    m_isRepeated = repeated;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    const GLint textureWrapParam = m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapParam));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapParam));
}


////////////////////////////////////////////////////////////
bool Texture::isRepeated() const
{
    return m_isRepeated;
}


////////////////////////////////////////////////////////////
bool Texture::generateMipmap()
{
    SFML_BASE_ASSERT(m_texture);
    SFML_BASE_ASSERT(glCheck(glIsTexture(m_texture)));

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glGenerateMipmap(GL_TEXTURE_2D));
    glCheck(glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER,
                            m_isSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR));

    m_hasMipmap = true;

    return true;
}


////////////////////////////////////////////////////////////
void Texture::invalidateMipmap()
{
    if (!m_hasMipmap)
        return;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));

    m_hasMipmap = false;
}


////////////////////////////////////////////////////////////
void Texture::bind() const
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());
    SFML_BASE_ASSERT(m_texture);

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
}


////////////////////////////////////////////////////////////
void Texture::unbind()
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

////////////////////////////////////////////////////////////
unsigned int Texture::getMaximumSize()
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalOrSharedGlContext());

    static const auto size = static_cast<unsigned int>(priv::getGLInteger(GL_MAX_TEXTURE_SIZE));
    return size;
}


////////////////////////////////////////////////////////////
Texture& Texture::operator=(const Texture& right)
{
    Texture temp(right);

    swap(temp);

    return *this;
}


////////////////////////////////////////////////////////////
void Texture::swap(Texture& right) noexcept
{
    std::swap(m_size, right.m_size);
    std::swap(m_texture, right.m_texture);
    std::swap(m_isSmooth, right.m_isSmooth);
    std::swap(m_sRgb, right.m_sRgb);
    std::swap(m_isRepeated, right.m_isRepeated);
    std::swap(m_fboAttachment, right.m_fboAttachment);
    std::swap(m_hasMipmap, right.m_hasMipmap);
    std::swap(m_cacheId, right.m_cacheId);
}


////////////////////////////////////////////////////////////
unsigned int Texture::getNativeHandle() const
{
    return m_texture;
}


////////////////////////////////////////////////////////////
FloatRect Texture::getRect() const
{
    return {{0, 0}, getSize().toVector2f()};
}


////////////////////////////////////////////////////////////
void swap(Texture& lhs, Texture& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace sf
