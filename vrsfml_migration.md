# SFML to VRSFML Migration Guide



## Headers

- Catch-all headers such as `<SFML/Graphics.hpp>` have been removed.

- Include exactly what you need instead, e.g.
    `#include <SFML/Graphics/Sprite.hpp>`



## Global Context State

- Global context state for graphics and audio is not managed automatically by SFML anymore.

- At the beginning of your main, create the appropriate context objects. They must live for the duration of the whole program.

```cpp
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    // ...rest of your application...
}
```



## Designated Initializer Configurations

- Most objects are now initalized using C++20's designated initialized syntax.

```cpp
//
// BEFORE (upstream SFML)
sf::RenderWindow window(
    sf::VideoMode({static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight)}, 32),
    "SFML Tennis",
    sf::Style::Titlebar | sf::Style::Close);

window.setVerticalSyncEnabled(true);

//
// AFTER (VRSFML)
sf::RenderWindow window({.size = gameSize.toVector2u(),
                         .bitsPerPixel = 32u,
                         .title = "SFML Tennis",
                         .resizable = false,
                         .vsync = true});
```

- This affects a lot of types, such as `sf::Text`, shapes, sprites, and so on.

```cpp
//
// BEFORE (upstream SFML)
sf::CircleShape ball;
ball.setRadius(ballRadius - 3);
ball.setOutlineThickness(2);
ball.setOutlineColor(sf::Color::Black);
ball.setFillColor(sf::Color::White);
ball.setOrigin({ballRadius / 2.f, ballRadius / 2.f});

//
// AFTER (VRSFML)
sf::CircleShape ball{{.origin           = {ballRadius / 2.f, ballRadius / 2.f},
                        .fillColor        = sf::Color::White,
                        .outlineColor     = sf::Color::Black,
                        .outlineThickness = 2.f,
                        .radius           = ballRadius - 3.f}};
```



## Standard Library Replacements

- To optimize compilation speed and debug run-time performance, VRSFML uses custom types instead of Standard Sibrary ones.
    - Check out the `SFML/Base` module to see all of them.

- VRSFML types use the `pascalCase` convention, and some APIs might be very different (or missing).

- You can still use Standard Library types if you want, but you might have to convert them in some VRSFML APIs.
    - However, you'll see a drastic compilation time improvement and debug performance improvement if you choose to use VRSFML types (and do not include the Standard headers).

Non-exhaustive table:

| Standard Library   | VRSFML                  |
|--------------------|-------------------------|
| `std::optional`    | `sf::Base::Optional`    |
| `std::unique_ptr`  | `sf::Base::UniquePtr`   |
| `std::string_view` | `sf::Base::StringView`  |
| `std::array`       | `sf::Base::Array`       |
| `std::span`        | `sf::Base::Span`        |
| `assert(...)`      | `SFML_BASE_ASSERT(...)` |
| `std::uint8_t`     | `sf::Base::U8`          |
| `std::uint16_t`    | `sf::Base::U16`         |
| `std::uint32_t`    | `sf::Base::U32`         |
| `std::int8_t`      | `sf::Base::I8`          |
| `std::int16_t`     | `sf::Base::I16`         |
| `std::int32_t`     | `sf::Base::I32`         |
| `std::size_t`      | `sf::Base::SizeT`       |



## Aggregatification Of Types

- Many types in VRSFML are now aggregate types (everything is public, no constructors).
    - This improves usage syntax, simplifies the implementation, and improves debug performance / compilation times.

- Notably, `sf::Transformable` is now an aggregate. Every transformable object will be affected.

```cpp
//
// BEFORE (upstream SFML)
if (ball.getPosition().y - ballRadius < 0.f)
{
    ballAngle = -ballAngle;
    ball.setPosition({ball.getPosition().x, ballRadius + 0.1f});
}
else if (ball.getPosition().y + ballRadius > gameHeight)
{
    ballAngle = -ballAngle;
    ball.setPosition({ball.getPosition().x, gameHeight - ballRadius - 0.1f});
}

//
// AFTER (VRSFML)
if (ball.position.y - ballRadius < 0.f)
{
    ballAngle       = -ballAngle;
    ball.position.y = ballRadius + 0.1f;
}
else if (ball.position.y + ballRadius > gameSize.y)
{
    ballAngle       = -ballAngle;
    ball.position.y = gameSize.y - ballRadius - 0.1f;
}
```



## Audio Multi-Device Support

- Playing sounds/musics now requires an audio device to be specified.

```cpp
//
// BEFORE (upstream SFML)
int main()
{
    // Load sounds
    const sf::SoundBuffer ballSoundBuffer(resourcesDir() / "ball.wav");
    sf::Sound             ballSound(ballSoundBuffer);

    // Play sound
    ballSound.play();
}

//
// AFTER (VRSFML)
int main()
{
    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    // Load sounds
    const auto ballSoundBuffer = sf::SoundBuffer::loadFromFile(resourcesDir() / "ball.wav").value();
    sf::Sound  ballSound(ballSoundBuffer);

    // Play sound
    ballSound.play(playbackDevice);
}
```



## Optional-Based Factory Functions

- Creation of objects/resources that can fail is done through factory functions returing an `sf::Base::Optional`.
    - This ensures that the user decides how to handle the failure case.

```cpp
//
// BEFORE (upstream SFML)
const sf::Texture sfmlLogoTexture(resourcesDir() / "sfml_logo.png");

//
// AFTER (VRSFML)
const auto sfmlLogoTexture = sf::Texture::loadFromFile(resourcesDir() / "sfml_logo.png").value();
```

- This change applies to all resources: textures, sound buffers, music, fonts, images, etc...



## Sprites And Shape Are Textureless

- Sprites and shapes do not point to a texture anymore.
    - The texture is specified during the draw call.

- This improves performance by shrinking the size of these objects (better cache friendliness).

- It also greatly improves safety by preventing dangling texture pointers.

- `nullptr` can be specified if a texture is not used (for the case of shapes).

```cpp
//
// BEFORE (upstream SFML)
const sf::Texture sfmlLogoTexture(resourcesDir() / "sfml_logo.png");
sf::Sprite        sfmlLogo(sfmlLogoTexture);
sfmlLogo.setPosition({170.f, 50.f});
// ...
window.draw(sfmlLogo);
window.draw(leftPaddle);
window.draw(rightPaddle);
window.draw(ball);

//
// AFTER (VRSFML)
const auto sfmlLogoTexture = sf::Texture::loadFromFile(resourcesDir() / "sfml_logo.png").value();
const sf::Sprite sfmlLogo({.position = {170.f, 50.f}});
// ...
window.draw(sfmlLogo, {.texture = &sfmlLogoTexture});
window.draw(leftPaddle);
window.draw(rightPaddle);
window.draw(ball);
```

Note that textures can be drawn directly in VRSFML, without the need of using a sprite:

```cpp
window.draw(sfmlLogoTexture, {.position = {170.f, 50.f}});
```



## Shader Uniform Location API

- Shader uniform locations are now retrieved via handles, preventing repeated string lookups.

```cpp
//
// BEFORE (upstream SFML)
shader.setUniform("texture", sf::Shader::CurrentTexture);
shader.setUniform("pixel_threshold", (x + y) / 30);

//
// AFTER (VRSFML)
auto ulTexture = shader.getUniformLocation("sf_u_texture").value(); // cache this
auto ulPixelThreshold = shader.getUniformLocation("pixel_threshold").value(); // cache this

shader.setUniform(ulTexture, sf::Shader::CurrentTexture);
shader.setUniform(ulPixelThreshold, (x + y) / 30);
```



## `sf::VertexArray` Is Gone

- Just use a `std::vector` or an array.
    - The primitive type is specified on the draw call.



## `sf::Drawable` Is Gone

- `sf::RenderTarget` has a template member function that accepts anything that exposes `.draw()`.

- Need polymorphism?
    - You don't.

- *Really* need polymorphism?
    - Create a polymorphic wrapper yourself (e.g. use type erasure) and then pass that wrapper to `sf::RenderTarget`.
        - This is trivial to do with `std::function`.



## Socket Blocking Behavior Must Be Specified At Construction Time

- Self-explanatory.

```cpp
//
// BEFORE (upstream SFML)
sf::TcpSocket socket;

//
// AFTER (VRSFML)
sf::TcpSocket socket(/* isBlocking */ true);
```



## Shader GLSL API Is Different

- Check `SFML/Graphics/GraphicsContext.cpp` to see the default shaders.

For your convenience:

```glsl
// DEFAULT VERTEX SHADER

layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform sampler2D sf_u_texture;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
    sf_v_texCoord = sf_a_texCoord / vec2(textureSize(sf_u_texture, 0));
}
```

```glsl
// DEFAULT FRAGMENT SHADER

layout(location = 1) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * texture(sf_u_texture, sf_v_texCoord);
}
```



## Batching

- Batching is not automatically done yet, but planned.
    - "Manual" batching is very easy to do.
    - Batch objects are similar to render targets, but not as generic.

```cpp
sf::CPUDrawableBatch batch;                 // uses CPU buffers
/* sf::PersistentGPUDrawableBatch batch; */ // uses persistent GPU buffer, not available on OpenGL ES

batch.clear();

batch.add(someSprite, someAssociatedTexture);
batch.add(someShape, someAssociatedTexture);
batch.add(someText); // associated texture is stored in the text object

window.draw(batch, commonRenderStates);
```

- Use texture atlases to batch multiple sprites/texts with different textures:

```cpp
sf::TextureAtlas atlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

const sf::FloatRect txrSpriteA = atlas.add(sf::Image::loadFromFile("spriteA.png").value()).value();
const sf::FloatRect txrSpriteB = atlas.add(sf::Image::loadFromFile("spriteB.png").value()).value();

const auto fontTuffy = sf::Font::openFromFile("resources/tuffy.ttf", &atlas).value();

batch.clear();
// ... add sprites, shapes, texts, etc to batch ...
window.draw(batch, {.texture = &atlas.getTexture()});
```



## Joystick Query API

TODO P0:
