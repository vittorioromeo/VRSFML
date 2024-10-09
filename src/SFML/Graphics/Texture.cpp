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
#include "SFML/Base/TrivialVector.hpp"

#ifdef SFML_OPENGL_ES
#include "SFML/Base/PtrDiffT.hpp"
#endif

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
Texture::Texture(base::PassKey<Texture>&&, GraphicsContext& graphicsContext, Vector2u size, unsigned int texture, bool sRgb) :
m_graphicsContext(&graphicsContext),
m_size(size),
m_texture(texture),
m_sRgb(sRgb),
m_cacheId(TextureImpl::getUniqueId())
{
}


////////////////////////////////////////////////////////////
Texture::Texture(const Texture& rhs) :
m_graphicsContext(rhs.m_graphicsContext),
m_isSmooth(rhs.m_isSmooth),
m_sRgb(rhs.m_sRgb),
m_isRepeated(rhs.m_isRepeated),
m_cacheId(TextureImpl::getUniqueId())
{
    base::Optional texture = create(*m_graphicsContext, rhs.getSize(), rhs.isSrgb());

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    const GLuint texture = m_texture;
    glCheck(glDeleteTextures(1, &texture));
}


////////////////////////////////////////////////////////////
Texture::Texture(Texture&& right) noexcept :
m_graphicsContext(right.m_graphicsContext),
m_size(base::exchange(right.m_size, {})),
m_texture(base::exchange(right.m_texture, 0u)),
m_isSmooth(base::exchange(right.m_isSmooth, false)),
m_sRgb(base::exchange(right.m_sRgb, false)),
m_isRepeated(base::exchange(right.m_isRepeated, false)),
m_pixelsFlipped(base::exchange(right.m_pixelsFlipped, false)),
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
        SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

        const GLuint texture = m_texture;
        glCheck(glDeleteTextures(1, &texture));
    }

    // Move old to new.
    m_graphicsContext = right.m_graphicsContext;

    m_size          = base::exchange(right.m_size, {});
    m_texture       = base::exchange(right.m_texture, 0u);
    m_isSmooth      = base::exchange(right.m_isSmooth, false);
    m_sRgb          = base::exchange(right.m_sRgb, false);
    m_isRepeated    = base::exchange(right.m_isRepeated, false);
    m_pixelsFlipped = base::exchange(right.m_pixelsFlipped, false);
    m_fboAttachment = base::exchange(right.m_fboAttachment, false);
    m_hasMipmap     = base::exchange(right.m_hasMipmap, false);
    m_cacheId       = base::exchange(right.m_cacheId, 0u);

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::create(GraphicsContext& graphicsContext, Vector2u size, bool sRgb)
{
    base::Optional<Texture> result; // Use a single local variable for NRVO

    // Check if texture parameters are valid before creating it
    if ((size.x == 0) || (size.y == 0))
    {
        priv::err() << "Failed to create texture, invalid size (" << size.x << "x" << size.y << ")";
        return result; // Empty optional
    }

    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    // Check the maximum texture size
    const unsigned int maxSize = getMaximumSize(graphicsContext);
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
    result.emplace(base::PassKey<Texture>{}, graphicsContext, size, glTexture, sRgb);
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

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromFile(GraphicsContext& graphicsContext, const Path& filename, bool sRgb, const IntRect& area)
{
    if (const base::Optional image = sf::Image::loadFromFile(filename))
        return loadFromImage(graphicsContext, *image, sRgb, area);

    priv::err() << "Failed to load texture from file";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromMemory(
    GraphicsContext& graphicsContext,
    const void*      data,
    base::SizeT      size,
    bool             sRgb,
    const IntRect&   area)
{
    if (const base::Optional image = sf::Image::loadFromMemory(data, size))
        return loadFromImage(graphicsContext, *image, sRgb, area);

    priv::err() << "Failed to load texture from memory";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromStream(GraphicsContext& graphicsContext, InputStream& stream, bool sRgb, const IntRect& area)
{
    if (const base::Optional image = sf::Image::loadFromStream(stream))
        return loadFromImage(graphicsContext, *image, sRgb, area);

    priv::err() << "Failed to load texture from stream";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Texture> Texture::loadFromImage(GraphicsContext& graphicsContext, const Image& image, bool sRgb, const IntRect& area)
{
    base::Optional<Texture> result; // Use a single local variable for NRVO

    // Retrieve the image size
    const auto size = image.getSize().toVector2i();

    // Load the entire image if the source area is either empty or contains the whole image
    if (area.size.x == 0 || (area.size.y == 0) ||
        ((area.position.x <= 0) && (area.position.y <= 0) && (area.size.x >= size.x) && (area.size.y >= size.y)))
    {
        // Load the entire image
        if ((result = sf::Texture::create(graphicsContext, image.getSize(), sRgb)))
        {
            result->update(image);
            return result;
        }

        // Error message generated in called function.
        return result; // Empty optional
    }

    // Load a sub-area of the image

    // Adjust the rectangle to the size of the image
    IntRect rectangle    = area;
    rectangle.position.x = base::max(rectangle.position.x, 0);
    rectangle.position.y = base::max(rectangle.position.y, 0);
    rectangle.size.x     = base::min(rectangle.size.x, size.x - rectangle.position.x);
    rectangle.size.y     = base::min(rectangle.size.y, size.y - rectangle.position.y);

    // Create the texture and upload the pixels
    if ((result = sf::Texture::create(graphicsContext, rectangle.size.toVector2u(), sRgb)))
    {
        SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    // Create an array of pixels
    base::TrivialVector<base::U8> pixels(m_size.x * m_size.y * 4);

#ifdef SFML_OPENGL_ES

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

        if (m_pixelsFlipped)
        {
            // Flip the texture vertically
            const auto stride             = static_cast<base::PtrDiffT>(m_size.x * 4);
            auto*      currentRowIterator = pixels.begin();
            auto*      nextRowIterator    = pixels.begin() + stride;
            auto*      reverseRowIterator = pixels.begin() + (stride * static_cast<base::PtrDiffT>(m_size.y - 1));
            for (unsigned int i = 0; i < m_size.y / 2; ++i)
            {
                base::swapRanges(currentRowIterator, nextRowIterator, reverseRowIterator);
                currentRowIterator = nextRowIterator;
                nextRowIterator += stride;
                reverseRowIterator -= stride;
            }
        }
    }

#else

    if (!m_pixelsFlipped)
    {
        // Texture is not padded nor flipped, we can use a direct copy
        glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
        glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()));
    }
    else
    {
        // Texture is either padded or flipped, we have to use a slower algorithm

        // All the pixels will first be copied to a temporary array
        base::TrivialVector<base::U8> allPixels(m_size.x * m_size.y * 4);

        glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
        glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, allPixels.data()));

        // Then we copy the useful pixels from the temporary array to the final one
        const base::U8*    src      = allPixels.data();
        base::U8*          dst      = pixels.data();
        auto               srcPitch = static_cast<int>(m_size.x * 4);
        const unsigned int dstPitch = m_size.x * 4;

        SFML_BASE_ASSERT(pixels.size() >= allPixels.size());

        // Handle the case where source pixels are flipped vertically
        if (m_pixelsFlipped)
        {
            src += static_cast<unsigned int>(srcPitch * static_cast<int>(m_size.y - 1));
            srcPitch = -srcPitch;
        }

        for (unsigned int i = 0; i < m_size.y; ++i)
        {
            SFML_BASE_MEMCPY(dst, src, dstPitch);
            src += srcPitch;
            dst += dstPitch;
        }
    }

#endif // SFML_OPENGL_ES

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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
    m_hasMipmap     = false;
    m_pixelsFlipped = false;
    m_cacheId       = TextureImpl::getUniqueId();

    // Force an OpenGL flush, so that the texture data will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());
}


////////////////////////////////////////////////////////////
bool Texture::update(const Texture& texture)
{
    // Update the whole texture
    return update(texture, {0, 0});
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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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
        GLboolean scissorEnabled = GL_FALSE;
        glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));

        if (scissorEnabled == GL_TRUE)
            glCheck(glDisable(GL_SCISSOR_TEST));

        // Blit the texture contents from the source to the destination texture
        glCheck(glBlitFramebuffer(0,
                                  texture.m_pixelsFlipped ? static_cast<GLint>(texture.m_size.y) : 0,
                                  static_cast<GLint>(texture.m_size.x),
                                  texture.m_pixelsFlipped ? 0 : static_cast<GLint>(texture.m_size.y), // Source rectangle, flip y if source is flipped
                                  static_cast<GLint>(dest.x),
                                  static_cast<GLint>(dest.y),
                                  static_cast<GLint>(dest.x + texture.m_size.x),
                                  static_cast<GLint>(dest.y + texture.m_size.y), // Destination rectangle
                                  GL_COLOR_BUFFER_BIT,
                                  GL_NEAREST));

        // Re-enable scissor testing if it was previously enabled
        if (scissorEnabled == GL_TRUE)
            glCheck(glEnable(GL_SCISSOR_TEST));
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
    m_hasMipmap     = false;
    m_pixelsFlipped = false;
    m_cacheId       = TextureImpl::getUniqueId();

    // Force an OpenGL flush, so that the texture data will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());
    return success;
}


////////////////////////////////////////////////////////////
void Texture::update(const Image& image)
{
    // Update the whole texture
    update(image.getPixelsPtr(), image.getSize(), {0u, 0u});
}


////////////////////////////////////////////////////////////
void Texture::update(const Image& image, Vector2u dest)
{
    update(image.getPixelsPtr(), image.getSize(), dest);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Texture::update(const Window& window)
{
    return update(window, {0u, 0u});
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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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
        GLboolean scissorEnabled = GL_FALSE;
        glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));

        if (scissorEnabled == GL_TRUE)
            glCheck(glDisable(GL_SCISSOR_TEST));

        // Blit the texture contents from the source to the destination texture
        glCheck(glBlitFramebuffer(0,
                                  0,
                                  static_cast<GLsizei>(window.getSize().x),
                                  static_cast<GLsizei>(window.getSize().y), // Source rectangle, flip y if source is flipped
                                  static_cast<GLint>(dest.x),
                                  static_cast<GLint>(dest.y),
                                  static_cast<GLint>(dest.x + window.getSize().x),
                                  static_cast<GLint>(dest.y + window.getSize().y), // Destination rectangle
                                  GL_COLOR_BUFFER_BIT,
                                  GL_NEAREST));

        // Re-enable scissor testing if it was previously enabled
        if (scissorEnabled == GL_TRUE)
            glCheck(glEnable(GL_SCISSOR_TEST));
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
    m_hasMipmap     = false;
    m_pixelsFlipped = true;
    m_cacheId       = TextureImpl::getUniqueId();

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

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

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Make sure that the current texture binding will be preserved
    const priv::TextureSaver save;

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));

    m_hasMipmap = false;
}


////////////////////////////////////////////////////////////
void Texture::bind([[maybe_unused]] GraphicsContext& graphicsContext) const
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    SFML_BASE_ASSERT(m_texture);

    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
}


////////////////////////////////////////////////////////////
void Texture::unbind([[maybe_unused]] GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

////////////////////////////////////////////////////////////
unsigned int Texture::getMaximumSize([[maybe_unused]] GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    static const auto size = static_cast<unsigned int>(priv::getGLInteger(GL_MAX_TEXTURE_SIZE));
    return size;
}


////////////////////////////////////////////////////////////
Texture::Params Texture::getParams(CoordinateType coordinateType) const
{
    // If non-normalized coordinates (= pixels) are requested, we need to
    // setup scale factors that convert the range [0 .. size] to [0 .. 1]

    // If pixels are flipped we must invert the Y axis
    const float pixelFlippedMult = m_pixelsFlipped ? -1.f : 1.f;

    return {coordinateType == CoordinateType::Pixels ? 1.f / static_cast<float>(m_size.x) : 1.f,
            (coordinateType == CoordinateType::Pixels ? 1.f / static_cast<float>(m_size.y) : 1.f) * pixelFlippedMult,
            m_pixelsFlipped ? 1.f : 0.f};
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
    std::swap(m_pixelsFlipped, right.m_pixelsFlipped);
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
