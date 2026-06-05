#if 1


    #define GLAD_GL_IMPLEMENTATION

    #include "Zancle/Diagnostic/Assert.hpp"

    #include <SDL3/SDL.h>
    #include <gl.h>

    #include <iostream>

    #define assert ZA_ASSERT


    #define glCheck(...)                                                           \
        [](auto&& f)                                                               \
        {                                                                          \
            const unsigned int openGlError = ::glGetError();                       \
                                                                                   \
            f();                                                                   \
                                                                                   \
            while (!::glCheckError(openGlError, __FILE__, __LINE__, #__VA_ARGS__)) \
                /* no-op */;                                                       \
        }([&]() { return __VA_ARGS__; })

bool glCheckError(const unsigned int openGlError, const char* const file, const unsigned int line, const char* const expression)
{
    const auto logError = [&](const char* const error, const char* const description)
    {
        std::cerr << "An internal OpenGL call failed in " << file << "(" << line << ")."
                  << "\nExpression:\n   " << expression << "\nError description:\n   " << error << "\n   "
                  << description << '\n';

        std::abort();
        return false;
    };

    switch (openGlError)
    {
        case GL_NO_ERROR:
            return true;

        case GL_INVALID_ENUM:
            return logError("GL_INVALID_ENUM", "An unacceptable value has been specified for an enumerated argument.");

        case GL_INVALID_VALUE:
            return logError("GL_INVALID_VALUE", "A numeric argument is out of range.");

        case GL_INVALID_OPERATION:
            return logError("GL_INVALID_OPERATION", "The specified operation is not allowed in the current state.");

        case GL_STACK_OVERFLOW:
            return logError("GL_STACK_OVERFLOW", "This command would cause a stack overflow.");

        case GL_STACK_UNDERFLOW:
            return logError("GL_STACK_UNDERFLOW", "This command would cause a stack underflow.");

        case GL_OUT_OF_MEMORY:
            return logError("GL_OUT_OF_MEMORY", "There is not enough memory left to execute the command.");

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return logError("GL_INVALID_FRAMEBUFFER_OPERATION",
                            "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".");

        default:
            return logError("Unknown error", "Unknown description");
    }
}

int main()
{
    assert(SDL_InitSubSystem(SDL_INIT_VIDEO));

    // SDL window associated with the shared OpenGL context
    SDL_Window* sharedCtxHiddenWindow = SDL_CreateWindow("", 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    assert(sharedCtxHiddenWindow);

    // SDL shared OpenGL context handle
    SDL_GLContext sharedCtx = SDL_GL_CreateContext(sharedCtxHiddenWindow);
    assert(sharedCtx);

    assert(SDL_GL_MakeCurrent(sharedCtxHiddenWindow, sharedCtx));
    gladLoadGL(&SDL_GL_GetProcAddress);

    //
    //
    // Setup globals and hidden shared OpenGL context (for resource sharing)
    // (uses `SDL_GLContext` internally)
    SDL_Window* window = SDL_CreateWindow("", 256, 256, SDL_WINDOW_OPENGL);
    assert(window);

    assert(SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1)); // Share shared context with next created context
    SDL_GLContext windowCtx = SDL_GL_CreateContext(window);
    assert(windowCtx);

    assert(SDL_GL_MakeCurrent(window, windowCtx));
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0u));

    //
    //
    // Create OpenGL texture on the shared context, then revert to previous context and bind it
    GLuint glTexture{};

    assert(SDL_GL_MakeCurrent(sharedCtxHiddenWindow, sharedCtx));

    glCheck(glGenTextures(1, &glTexture));
    assert(glTexture);

    assert(SDL_GL_MakeCurrent(window, windowCtx));
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0u));

    glCheck(glBindTexture(GL_TEXTURE_2D, glTexture));
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    //
    //
    // Set window context to active and calls `glClearColor` + `glClear`
    glCheck(glClearColor(1.f, 0.f, 0.f, 1.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
    // Intentionally not calling `SDL_GL_SwapWindow`

    //
    //
    // Create destination framebuffer and bind the previously created texture to it
    GLuint destFrameBuffer{};
    glCheck(glGenFramebuffers(1, &destFrameBuffer));
    assert(destFrameBuffer);

    // Read from the window FBO, write to the texture FBO
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0u /* default FBO */));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBuffer));

    glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture, 0));

    assert(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    //
    //
    // This context switching should be idempotent, but commenting out `sharedCtxPtr.makeCurrent(true)` FIXES THE ISSUE
    assert(SDL_GL_MakeCurrent(sharedCtxHiddenWindow, sharedCtx));
    assert(SDL_GL_MakeCurrent(window, windowCtx));
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0u));

    //
    //
    // This query should be a noop, but commenting out `glGetIntegerv` FIXES THE ISSUE
    GLint out{};
    glCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &out));

    // (!) EITHER (!) the context switching or the query must be commented out to FIX THE ISSUE

    //
    //
    // Blit the framebuffer from the window FBO to the texture FBO, then delete the framebuffer
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0u));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBuffer));
    glCheck(glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    glCheck(glDeleteFramebuffers(1, &destFrameBuffer));

    //
    //
    // Write texture data into a pixel array and read it back
    glCheck(glBindTexture(GL_TEXTURE_2D, glTexture));

    unsigned char pixels[256 * 256 * 4]{};

    GLuint imageFBO{};
    glCheck(glGenFramebuffers(1, &imageFBO));
    assert(imageFBO);

    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, imageFBO));
    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture, 0));
    glCheck(glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
    glCheck(glDeleteFramebuffers(1, &imageFBO));

    const auto           index = (64 + 64 * 256) * 4; // pixel at (64, 64)
    const unsigned char* pixel = &pixels[index];

    //
    //
    // These assertions will fail unless EITHER the context switching or the query is commented out
    assert(pixel[0] == 255); // Red
    assert(pixel[1] == 0);   // Green
    assert(pixel[2] == 0);   // Blue
    assert(pixel[3] == 255); // Alpha
}

#else

    #include "Zancle/Graphics/Color.hpp"
    #include "Zancle/Graphics/DrawableBatch.hpp"
    #include "Zancle/Graphics/Font.hpp"
    #include "Zancle/Graphics/GraphicsContext.hpp"
    #include "Zancle/Graphics/Image.hpp"
    #include "Zancle/Graphics/PrimitiveType.hpp"
    #include "Zancle/Graphics/RenderStates.hpp"
    #include "Zancle/Graphics/RenderTexture.hpp"
    #include "Zancle/Graphics/RenderWindow.hpp"
    #include "Zancle/Graphics/Shader.hpp"
    #include "Zancle/Graphics/Sprite.hpp"
    #include "Zancle/Graphics/Text.hpp"
    #include "Zancle/Graphics/Texture.hpp"
    #include "Zancle/Graphics/TextureAtlas.hpp"
    #include "Zancle/Graphics/Vertex.hpp"

    #include "Zancle/Window/EventUtils.hpp"
    #include "Zancle/Window/WindowContext.hpp"

int main()
{
    auto windowContext = za::WindowContext::create().value();

    za::RenderWindow rw{
        {.size            = {3440u, 1440u},
         .title           = "sus",
         .fullscreen      = false,
         .resizable       = false,
         .closable        = false,
         .hasTitlebar     = false,
         .vsync           = false,
         .frametimeLimit  = 60u,
         .contextSettings = {}}};

    while (true)
    {
        while (za::Optional event = rw.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        rw.clear(za::Color::Black);
        rw.display();
    }
}


    #include "Zancle/ImGui/ImGui.hpp"

    #include "Zancle/Graphics/Color.hpp"
    #include "Zancle/Graphics/DrawableBatch.hpp"
    #include "Zancle/Graphics/Font.hpp"
    #include "Zancle/Graphics/GraphicsContext.hpp"
    #include "Zancle/Graphics/Image.hpp"
    #include "Zancle/Graphics/PrimitiveType.hpp"
    #include "Zancle/Graphics/RenderStates.hpp"
    #include "Zancle/Graphics/RenderTexture.hpp"
    #include "Zancle/Graphics/RenderWindow.hpp"
    #include "Zancle/Graphics/Shader.hpp"
    #include "Zancle/Graphics/Sprite.hpp"
    #include "Zancle/Graphics/Text.hpp"
    #include "Zancle/Graphics/Texture.hpp"
    #include "Zancle/Graphics/TextureAtlas.hpp"
    #include "Zancle/Graphics/Vertex.hpp"

    #include "Zancle/Window/EventUtils.hpp"

    #include "Zancle/IO/Path.hpp"
    #include "Zancle/System/Vector2.hpp"

    #include "Zancle/Vocabulary/Optional.hpp"

    #define IMGUI_DEFINE_MATH_OPERATORS
    #include <imgui.h>


////////////////////////////////////////////////////////////

    #if 1

int main()
{
    auto             graphicsContext = za::GraphicsContext::create().value();
    za::RenderWindow window({.size{800u, 600u}, .title = L"महसुस", .contextSettings = {.antiAliasingLevel = 4}});


    const float width     = 128.f;
    const float height    = 64.f;
    const float halfWidth = width / 2.f;

    const za::Vector2u size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};

    const auto     font0 = za::Font::openFromFile("resources/tuffy.ttf").value();
    const za::Text text0(font0, {.position = {0u, 0u}, .string = "Test", .characterSize = 16u});

    auto image   = za::Image::create(size, za::Color::White).value();
    auto texture = za::Texture::loadFromImage(image).value();

    auto baseRenderTexture = za::RenderTexture::create(size, {.antiAliasingLevel = 0, .sRgbCapable = true}).value();

    auto baseRenderTextureAA = za::RenderTexture::create(size, {.antiAliasingLevel = 4, .sRgbCapable = true}).value();

    auto leftInnerRT = za::RenderTexture::create(size, {.antiAliasingLevel = 4, .sRgbCapable = true}).value();

    const za::Vertex leftVertexArray[6]{{{0.f, 0.f}, za::Color::Red, {0.f, 0.f}},
                                        {{halfWidth, 0.f}, za::Color::Red, {halfWidth, 0.f}},
                                        {{0.f, height}, za::Color::Red, {0.f, height}},
                                        {{0.f, height}, za::Color::Green, {0.f, height}},
                                        {{halfWidth, 0.f}, za::Color::Green, {halfWidth, 0.f}},
                                        {{halfWidth, height}, za::Color::Green, {halfWidth, height}}};

    leftInnerRT.clear();
    leftInnerRT.draw(texture);


    leftInnerRT.display();


    auto winRT = za::Texture::create(window.getSize()).value();

        // auto finalImage = baseRenderTexture.getTexture().copyToImage();
        // auto finalTx    = za::Texture::loadFromImage(finalImage).value();

        #define CHECK(...)                                  \
            if (!(__VA_ARGS__))                             \
            {                                               \
                za::cOut() << "fail " #__VA_ARGS__ << '\n'; \
            }

    za::Sprite rtSprite{.textureRect = baseRenderTexture.getTexture().getRect()};
    za::Sprite rtAASprite{.textureRect = baseRenderTextureAA.getTexture().getRect()};

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;
        }

        window.clear();

        const auto doit = [&](auto& rt, auto& rts, float xBias)
        {
            rt.clear();
            rt.draw(leftVertexArray, za::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});
            rt.display();

            rts.position = {xBias, 0};
            window.draw(rts, {.texture = &rt.getTexture()});

            rt.clear();
            rt.draw(leftVertexArray, za::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});
            rt.display();

            rts.position = {xBias + 128, 0};
            window.draw(rts, {.texture = &rt.getTexture()});

            rt.clear();
            rt.draw(leftVertexArray, za::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});
            rt.draw(text0);
            rt.display();

            rts.position = {xBias, 128};
            window.draw(rts, {.texture = &rt.getTexture()});
        };

        doit(baseRenderTexture, rtSprite, 0);
        doit(baseRenderTextureAA, rtAASprite, 256);

        bool rc = winRT.update(window, {});
        if (!rc)
            throw 100;

        window.draw(winRT, {.position = {256, 256}, .scale = {0.2f, 0.2f}});
        window.display();
    }
}

    #elif 0

int main()
{
    auto             graphicsContext = za::GraphicsContext::create().value();
    za::RenderWindow window({.size{800u, 600u}, .title = L"महसुस", .contextSettings = {.antiAliasingLevel = 4}});


    za::Vector2u size = window.getSize();

    auto texture = za::Texture::loadFromFile("resources/biga.png").value();

    za::Sprite sprite{.textureRect = texture.getRect()};

    sprite.scale = {(float)size.x / texture.getSize().x, (float)size.y / texture.getSize().y / 2.f};

    auto render = za::RenderTexture::create({size.x, (unsigned int)(size.y / 2.f)}, {.antiAliasingLevel = 4}).value();

    za::Sprite rndrSprite{.textureRect = render.getTexture().getRect()};
    rndrSprite.position = {0.f, (float)size.y / 2.f};

    const char* shaderSrc = R"glsl(
// This shader draws texture on the left using RB channels and
// texture2 on the right using G channel
layout(location = 1) uniform sampler2D za_u_texture;
uniform sampler2D texture2;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
	vec4 color1 = texture( za_u_texture, za_v_texCoord * vec2( 2.0, 1.0 ) + vec2( 0.0, 0.0 ));
	vec4 color2 = texture( texture2, za_v_texCoord * vec2( 2.0, 1.0 ) + vec2( -1.0, 0.0 ));
	za_fragColor = za_v_color * ( vec4(color2.x, color1.y, color2.z, 1.0 ));
}
)glsl";

    auto shader     = za::Shader::loadFromMemory({.fragmentCode = shaderSrc}).value();
    auto ulTexture2 = shader.getUniformLocation("texture2").value();
    (void)shader.setUniform(ulTexture2, texture);

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        render.clear();
        render.draw(sprite, texture);
        render.display();

        window.clear();
        window.draw(sprite, texture, {.shader = &shader});
        window.draw(rndrSprite, render.getTexture(), {.shader = &shader});
        window.display();
    }

    return 0;
}

    #elif 1

int main()
{
    auto             graphicsContext = za::GraphicsContext::create().value();
    za::RenderWindow window({.size{800u, 600u}, .title = L"महसुस"});

    za::RectangleShape rs0(
        {.position         = {250.f, 250.f},
         .origin           = {0.f, 0.f},
         .fillColor        = za::Color::Red,
         .outlineColor     = za::Color::Yellow,
         .outlineThickness = 3.f,
         .size             = {64.f, 64.f}});

    za::RectangleShape cs0(
        {.position         = {450.f, 450.f},
         .origin           = {-25.f, 50.f},
         .fillColor        = za::Color::Blue,
         .outlineColor     = za::Color::Yellow,
         .outlineThickness = 2.f,
         .size             = {36.f, 36.f}});

    rs0.setTopRight(window.getTopRight());

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        window.draw(rs0);
        window.draw(cs0);

        cs0.rotation += za::radians(0.0005f);
        cs0.setCenter(rs0.getBottomLeft());

        window.display();
    }
}

    #elif 0

int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    za::RenderWindow window({.size{800u, 600u}, .title = L"महसुस"});

    auto textureAtlas = za::TextureAtlas{za::Texture::create({1024u, 1024u}).value()};

    const auto font0 = za::Font::openFromFile("resources/tuffy.ttf", &textureAtlas).value();
    const auto font1 = za::Font::openFromFile("resources/mouldycheese.ttf", &textureAtlas).value();

    const auto zancleLogoImage     = za::Image::loadFromFile("resources/sfml_logo.png").value();
    const auto zancleLogoAtlasRect = textureAtlas.add(zancleLogoImage).value();

    const auto whiteDotAtlasRect = textureAtlas.add(graphicsContext.getBuiltInWhiteDotTexture()).value();

    za::Sprite zancleLogo(zancleLogoAtlasRect);

    const za::Text text0(font0, {.position = {0u, 0u}, .string = "Test", .characterSize = 128u});
    const za::Text text1(font0, {.position = {128u, 0u}, .string = "acbasdfbFOOBAR", .characterSize = 32u});
    const za::Text text2(font0, {.position = {0u, 128u}, .string = "ssdfbsdbfudsy", .characterSize = 64u});
    const za::Text text3(font1, {.position = {128u, 128u}, .string = "Test", .characterSize = 128u});
    const za::Text text4(font1, {.position = {256u, 128u}, .string = "FOmfgj,ryfkmtdfOBAR", .characterSize = 32u});
    const za::Text text5(font1, {.position = {128u, 256u}, .string = "abscas", .characterSize = 64u});

    const za::CircleShape circle0{
        {.position           = {350.f, 350.f},
         .scale              = {2.f, 2.f},
         .textureRect        = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
         .outlineTextureRect = {.position = whiteDotAtlasRect.position, .size{0.f, 0.f}},
         .fillColor          = za::Color::Red,
         .outlineColor       = za::Color::Yellow,
         .outlineThickness   = 8.f,
         .radius             = 45.f}};

    // Create drawable batch to optimize rendering
    za::CPUDrawableBatch drawableBatch;

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        {
            drawableBatch.clear();

            drawableBatch.add(text0);

            zancleLogo.position = {170.f, 50.f};
            zancleLogo.scale    = {1.5f, 1.5f};
            drawableBatch.add(zancleLogo);

            drawableBatch.add(text1);
            zancleLogo.position = {100.f, 50.f};
            zancleLogo.scale    = {1.f, 1.f};
            drawableBatch.add(zancleLogo);

            drawableBatch.add(text2);
            zancleLogo.position = {300.f, 150.f};
            zancleLogo.scale    = {1.5f, 1.5f};
            drawableBatch.add(zancleLogo);

            drawableBatch.add(text3);
            zancleLogo.position = {250.f, 250.f};
            zancleLogo.scale    = {1.f, 1.f};
            drawableBatch.add(zancleLogo);

            drawableBatch.add(text4);
            drawableBatch.add(text5);

            drawableBatch.add(circle0);

            window.draw(drawableBatch, {.texture = &textureAtlas.getTexture()});
        }

        // window.draw(circle0);

        window.display();
    }

    return -100;
}

    #elif defined(FOOOO)

int main()
{
    const float screenWidth  = 800.f;
    const float screenHeight = 600.f;

    const za::Vector2u screenSize{static_cast<unsigned int>(screenWidth), static_cast<unsigned int>(screenHeight)};

    auto graphicsContext = za::GraphicsContext::create().value();

    za::cOut() << za::Texture::getMaximumSize() << '\n';
    return 0;

    // TODO P0: aa level of 4 causes glcheck assert fail on opengl

    za::RenderWindow window({.size{screenSize},
                             .title = "Window",
                             .vsync = true,
                             .contextSettings{.depthBits = 0, .stencilBits = 0, .antiAliasingLevel = 4}});

    auto image   = za::Image::create(screenSize, za::Color::White).value();
    auto texture = za::Texture::loadFromImage(image).value();

    auto baseRenderTexture = za::RenderTexture::create(screenSize, za::ContextSettings{0, 0, 4 /* AA level*/}).value();

    za::RenderTexture
        renderTextures[2]{za::RenderTexture::create(screenSize, za::ContextSettings{0, 0, 4 /* AA level*/}).value(),
                          za::RenderTexture::create(screenSize, za::ContextSettings{0, 0, 4 /* AA level*/}).value()};

    std::vector<za::Vertex> vertexArrays[2];

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        vertexArrays[0].clear();
        vertexArrays[1].clear();

        float xCenter = screenWidth / 2;

        vertexArrays[0].emplace_back(za::Vector2f{0, 0}, za::Color::White, za::Vector2f{0, 0});
        vertexArrays[0].emplace_back(za::Vector2f{xCenter, 0}, za::Color::White, za::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(za::Vector2f{0, screenHeight}, za::Color::White, za::Vector2f{0, screenHeight});

        vertexArrays[0].emplace_back(za::Vector2f{0, screenHeight}, za::Color::White, za::Vector2f{0, screenHeight});
        vertexArrays[0].emplace_back(za::Vector2f{xCenter, 0}, za::Color::White, za::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(za::Vector2f{xCenter, screenHeight}, za::Color::White, za::Vector2f{xCenter, screenHeight});

        // right half of screen
        vertexArrays[1].emplace_back(za::Vector2f{xCenter, 0}, za::Color::White, za::Vector2f{xCenter, 0});
        vertexArrays[1].emplace_back(za::Vector2f{screenWidth, 0}, za::Color::White, za::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(za::Vector2f{xCenter, screenHeight}, za::Color::White, za::Vector2f{xCenter, screenHeight});

        vertexArrays[1].emplace_back(za::Vector2f{xCenter, screenHeight}, za::Color::White, za::Vector2f{xCenter, screenHeight});
        vertexArrays[1].emplace_back(za::Vector2f{screenWidth, 0}, za::Color::White, za::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(za::Vector2f{screenWidth, screenHeight},
                                     za::Color::White,
                                     za::Vector2f{screenWidth, screenHeight});

        renderTextures[0].clear();
        renderTextures[1].clear();

        renderTextures[0].draw(texture);
        renderTextures[1].draw(texture, {.color = za::Color::Green});

        baseRenderTexture.clear();


        renderTextures[0].display();
        baseRenderTexture.draw(vertexArrays[0], za::PrimitiveType::Triangles, {.texture = &renderTextures[0].getTexture()});

        renderTextures[1].display();
        baseRenderTexture.draw(vertexArrays[1], za::PrimitiveType::Triangles, {.texture = &renderTextures[1].getTexture()});

        baseRenderTexture.display();

        window.draw(baseRenderTexture.getTexture());
        window.display();
    }

    return 0;
}

    #elif defined(BARABARAR)

        #include "Zancle/Graphics/Font.hpp"
        #include "Zancle/Graphics/GraphicsContext.hpp"
        #include "Zancle/Graphics/RenderTexture.hpp"
        #include "Zancle/Graphics/Text.hpp"

        #include "Zancle/IO/Path.hpp"
        #include "Zancle/System/String.hpp"

int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    za::RenderWindow window({.size{800u, 600u}, .title = "Test", .vsync = false, .resizable = false});

    const auto font = za::Font::openFromFile("resources/tuffy.ttf").value();

    za::Text text(font, "Test", 20);

    za::RenderTexture renderTexture[10]{za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value(),
                                        za::RenderTexture::create({800u, 600u}).value()};

    ;

    za::Clock          clock;
    std::ostringstream oss;

    while (true)
    {
        while (za::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        for (int i = 0; i < 20; ++i)
        {
            for (auto& j : renderTexture)
            {
                j.clear(za::Color(0, 0, 0));

                for (auto& u : renderTexture)
                    u.draw(text);

                j.display();
            }

            window.clear(za::Color(0, 0, 0));

            for (const auto& j : renderTexture)
            {
                for (int k = 0; k < 10; ++k)
                    window.draw(j.getTexture());
            }
        }

        window.display();

        oss.str("");
        oss << "Test -- Frame: " << clock.restart().asSeconds() << " sec";

        window.setTitle(oss.str());
    }

    return EXIT_SUCCESS;
}

    #else

        #include "Zancle/Graphics/Font.hpp"
        #include "Zancle/Graphics/GraphicsContext.hpp"
        #include "Zancle/Graphics/RenderTexture.hpp"
        #include "Zancle/Graphics/Text.hpp"

        #include "Zancle/IO/Path.hpp"
        #include "Zancle/System/String.hpp"

        #include <cstdlib>

int main()
{
    auto graphicsContext = za::GraphicsContext::create().value();

    const auto       font         = za::Font::openFromFile("resources/tuffy.ttf").value();
    const za::String textContents = "abcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\n";

    auto text          = za::Text(font, textContents);
    auto renderTexture = za::RenderTexture::create({1680, 1050}).value();

    renderTexture.clear();

    for (std::size_t i = 0; i < 100'000; ++i)
    {
        text.setOutlineThickness(static_cast<float>(5 + (i % 2)));
        renderTexture.draw(text);
    }

    renderTexture.display();
}

    #endif

#endif
