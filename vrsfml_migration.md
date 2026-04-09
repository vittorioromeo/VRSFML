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
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // ...rest of your application...
}
```



## Designated Initializer Configurations

- Most objects are now initialized using C++20's designated initializer syntax.

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
sf::RenderWindow window({.size = gameSize.toVec2u(),
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

- To optimize compilation speed and debug run-time performance, VRSFML uses custom types instead of Standard Library ones.
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
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // Load sounds
    const auto ballSoundBuffer = sf::SoundBuffer::loadFromFile(resourcesDir() / "ball.wav").value();
    sf::Sound  ballSound(playbackDevice, ballSoundBuffer);

    // Play sound
    ballSound.play();
}
```



## Optional-Based Factory Functions

- Creation of objects/resources that can fail is done through factory functions returning an `sf::Base::Optional`.
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



## Sprites And Shapes Are Textureless

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
layout(location = 4) uniform vec2 sf_u_invTextureSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
    sf_v_texCoord = sf_a_texCoord * sf_u_invTextureSize;
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

- Manual batching is fully supported to minimize state changes and draw calls.
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

- Use texture atlases to batch multiple sprites/texts with different textures into a single draw call:

```cpp
sf::TextureAtlas atlas{sf::Texture::create({1024u, 1024u}, {.smooth = true}).value()};

const sf::Rect2f txrSpriteA = atlas.add(sf::Image::loadFromFile("spriteA.png").value()).value();
const sf::Rect2f txrSpriteB = atlas.add(sf::Image::loadFromFile("spriteB.png").value()).value();

const auto fontTuffy = sf::Font::openFromFile("resources/tuffy.ttf", &atlas).value();

batch.clear();
// ... add sprites, shapes, texts, etc to batch ...
window.draw(batch, {.texture = &atlas.getTexture()});
```



## Window `getPosition` Changes

Window `getPosition` now returns the top-left corner of the window's contents (not including titlebar, decorations, etc).



## Joystick Query API

- The joystick API has been redesigned from static class methods to a **query object pattern**.
    - Instead of calling `sf::Joystick::isButtonPressed(id, button)`, you first obtain an `Optional<Query>` handle for a specific joystick, then call methods on it.

```cpp
//
// BEFORE (upstream SFML)
if (sf::Joystick::isConnected(0))
{
    bool pressed = sf::Joystick::isButtonPressed(0, 2);
    float pos    = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
    unsigned int buttonCount = sf::Joystick::getButtonCount(0);

    sf::Joystick::Identification id = sf::Joystick::getIdentification(0);
    const auto& name = id.name;
}

//
// AFTER (VRSFML)
if (const auto query = sf::Joystick::query(0); query.hasValue())
{
    bool pressed = query->isButtonPressed(2);
    float pos    = query->getAxisPosition(sf::Joystick::Axis::Y);
    unsigned int buttonCount = query->getButtonCount();

    const auto& name     = query->getName();
    unsigned int vendorId = query->getVendorId();
}
```

- If `sf::Joystick::query(id)` returns `base::nullOpt`, the joystick is not connected.

- Event-based joystick input still works as before (via `sf::Event::JoystickButtonPressed`, etc.).

- If you have no window, call `sf::Joystick::update()` manually to refresh the cached state.



## SDL3 Backend

- VRSFML transitions to utilizing SDL3 as its backend for window creation, input handling, and platform-specific heavy lifting.
    - This drastically improves platform compatibility (Wayland native support, better controller mapping, smoother resize events) compared to upstream SFML's custom backend code.



## Volume Rescaling

Sound/music volume is now in `[0.0, 1.0]` range instead of `[0.0, 100.0]` range.

```cpp
//
// BEFORE (upstream SFML)
sound.setVolume(50.f); // Half volume

//
// AFTER (VRSFML)
sound.setVolume(0.5f); // Half volume
```



## Autobatching

- VRSFML features a powerful transparent autobatcher built into `sf::RenderTarget`.
    - If enabled, sequential draw calls sharing the same `RenderStates` (texture, shader, blend mode) are aggregated automatically and sent to the GPU in a single operation.
    - You do not need to manually manage `sf::CPUDrawableBatch` if you sort your draw calls by texture/state.

```cpp
// Assuming autobatch is enabled (default)
window.draw(sprite1, {.texture = &atlas.getTexture()});
window.draw(sprite2, {.texture = &atlas.getTexture()});
window.draw(sprite3, {.texture = &atlas.getTexture()});
// The above triggers only 1 OpenGL draw call!
```

- You can tune or disable this behavior via `window.setAutoBatchMode(...)`.



## High DPI Support

- VRSFML provides proper High DPI support via SDL3's display scaling infrastructure.

- Query the display scale factor at runtime with `window.getDisplayScale()`:

```cpp
//
// BEFORE (upstream SFML)
// No standard mechanism for high DPI support.

//
// AFTER (VRSFML)
const float scale = window.getDisplayScale();
// Returns 1.f for standard DPI (96), 2.f for Retina "@2x" displays, etc.
```

- Window sizes are always in **logical pixels**. The OS handles conversion to the physical framebuffer size automatically.

- You can also query the primary display's content scale without a window:

```cpp
const float displayScale = sf::VideoModeUtils::getPrimaryDisplayContentScale();
```

- `sf::VideoMode` includes a `pixelDensity` field reflecting the HiDPI scale for each video mode.

- VRSFML does **not** automatically scale your coordinates -- you must apply `getDisplayScale()` manually for UI elements, text sizing, etc.



## Windows Are Not Closable Anymore

- `sf::Window::isOpen()` has been removed. A window object's lifetime dictates its existence.
    - If you need to represent a window that might be closed or destroyed, wrap it in an `sf::Base::Optional<sf::RenderWindow>`.

```cpp
//
// BEFORE (upstream SFML)
sf::RenderWindow window(...);
while (window.isOpen())
{
    if (event.type == sf::Event::Closed)
        window.close();
}

//
// AFTER (VRSFML)
auto window = sf::RenderWindow::create(...).value();

while (true)
{
    while (const auto event = window.pollEvent())
        if (event->is<sf::Event::Closed>())
            return 0;
}

// Window is destroyed (closed) when it goes out of scope here.
```



## Type Renames

- The math vector types have been shortened to align with standard graphics terminology and for conciseness.

| Upstream SFML      | VRSFML        |
|--------------------|---------------|
| `sf::Vector2i`     | `sf::Vec2i`   |
| `sf::Vector2u`     | `sf::Vec2u`   |
| `sf::Vector2f`     | `sf::Vec2f`   |
| `sf::Vector3i`     | `sf::Vec3i`   |
| `sf::Vector3u`     | `sf::Vec3u`   |
| `sf::Vector3f`     | `sf::Vec3f`   |
| `sf::IntRect`      | `sf::Rect2i`  |
| `sf::FloatRect`    | `sf::Rect2f`  |

- Additional type aliases exist: `sf::Vec2uz`, `sf::Vec3uz` (for `base::SizeT` components), `sf::Rect2u`, `sf::Rect2uz`.

- Fixed-width integer types live in `sf::base`:

| Upstream SFML  | VRSFML         |
|----------------|----------------|
| `sf::Int8`     | `sf::base::I8` |
| `sf::Uint8`    | `sf::base::U8` |
| `sf::Int16`    | `sf::base::I16`|
| `sf::Uint16`   | `sf::base::U16`|
| `sf::Int32`    | `sf::base::I32`|
| `sf::Uint32`   | `sf::base::U32`|
| `sf::Int64`    | `sf::base::I64`|
| `sf::Uint64`   | `sf::base::U64`|



## Sound Is Non-Movable

- `sf::Sound` is neither copyable nor movable.
    - It maintains internal references to a `PlaybackDevice` and a `SoundBuffer`, and contains embedded miniaudio state that must remain at a stable memory address.

- To manage collections of sounds, use `sf::base::InPlaceVector` with `reEmplaceByIterator` to reuse slots:

```cpp
//
// Pool of up to 256 concurrent sounds
sf::base::InPlaceVector<sf::Sound, 256> sounds;

// Play a new sound by emplacing it at the end
sounds.emplaceBack(playbackDevice, buffer).play();

// Reuse a stopped slot for a new sound
auto* it = sf::base::findIf(sounds.begin(), sounds.end(),
    [](const sf::Sound& s) { return !s.isPlaying(); });

if (it != sounds.end())
    sounds.reEmplaceByIterator(it, playbackDevice, buffer).play();
```

- For a single optional sound, use `sf::base::Optional<sf::Sound>` with `.emplace(...)` to reconstruct in-place.



## Music Is Non-Movable

- `sf::Music` is neither copyable nor movable for the same reasons as `sf::Sound` (stable references, streaming thread).

- The same patterns apply: use `sf::base::Optional<sf::Music>` for single instances, or `sf::base::InPlaceVector` for collections.

```cpp
sf::base::Optional<sf::Music> bgMusic;

// Start playing
bgMusic.emplace(playbackDevice, musicReader);
bgMusic->play();

// Switch to a different track (destroys old, constructs new in-place)
bgMusic.emplace(playbackDevice, otherMusicReader);
bgMusic->play();
```



## MusicReader: Decoupled Music Source

- Music loading and playback are now split into two objects:
    - `sf::MusicReader` -- owns the audio file/stream/memory source, is **movable**.
    - `sf::Music` -- performs playback from a `MusicReader`, is **non-movable**.

- The `MusicReader` must outlive any `Music` instances that reference it.

```cpp
//
// BEFORE (upstream SFML)
sf::Music music;
music.openFromFile("music.ogg");
music.play();

//
// AFTER (VRSFML)
auto musicReader = sf::MusicReader::openFromFile("music.ogg").value();
sf::Music music(playbackDevice, musicReader);
music.play();
```

- This separation means you can keep the `MusicReader` alive and construct/destroy `Music` instances freely (e.g. to switch playback devices).



## ContextSettings Without Antialiasing And sRGB Support

- `sf::ContextSettings` no longer accepts `antialiasingLevel` or `sRgbCapable` for standard window creation.
    - Relying on the OS window manager for MSAA and sRGB is historically buggy and inconsistent across drivers.
    - Instead, VRSFML encourages rendering to an `sf::RenderTexture` created with `sf::RenderTextureCreateSettings` (where MSAA and sRGB are strictly controlled via FBOs), and blitting the final result to the window.



## CoordinateType Is Gone

- `sf::CoordinateType` has been removed. Texture coordinates in `sf::Vertex` are now **always in pixel units** (not normalized).

- The default vertex shader automatically normalizes pixel coordinates to `[0, 1]` via a precomputed inverse texture size uniform:

```glsl
sf_v_texCoord = sf_a_texCoord * sf_u_invTextureSize;
```

- If you had code that switched between `CoordinateType::Pixels` and `CoordinateType::Normalized`, simply remove those switches -- pixel coordinates are always used now.

```cpp
//
// BEFORE (upstream SFML)
sf::Vertex vertex;
vertex.texCoords = {0.5f, 0.5f}; // Could be normalized or pixels depending on CoordinateType

//
// AFTER (VRSFML)
sf::Vertex vertex;
vertex.texCoords = {128.f, 128.f}; // Always pixel coordinates (e.g. center of a 256x256 texture)
```

- If you write custom shaders, note that the vertex shader receives pixel-space texture coordinates and must normalize them (the default shader does this for you).



## Views Are Not Stateful Anymore

- Views are no longer stored as persistent state on `sf::RenderTarget`. Instead, they are passed **per draw call** via `sf::RenderStates`.

```cpp
//
// BEFORE (upstream SFML)
sf::View gameView({0.f, 0.f, 800.f, 600.f});
window.setView(gameView);
window.draw(sprite);  // uses gameView
window.draw(text);    // uses gameView

sf::View uiView = window.getDefaultView();
window.setView(uiView);
window.draw(button);  // uses uiView

//
// AFTER (VRSFML)
sf::View gameView = sf::View::fromRect({{0.f, 0.f}, {800.f, 600.f}});
window.draw(sprite, {.view = gameView, .texture = &texture});
window.draw(text,   {.view = gameView});

sf::View uiView = window.computeView(); // default view matching window size
window.draw(button, {.view = uiView});
```

- Use `window.computeView()` to get a default view matching the current window size (replaces `getDefaultView()`).

- Coordinate conversion now takes the target size as a parameter:

```cpp
//
// BEFORE (upstream SFML)
sf::Vec2f worldPos = window.mapPixelToCoords(mousePos, gameView);

//
// AFTER (VRSFML)
sf::Vec2f worldPos = gameView.screenToWorld(mousePos.toVec2f(), window.getSize().toVec2f());
sf::Vec2f screenPos = gameView.worldToScreen(entityPos, window.getSize().toVec2f());
```



## Event System Overhaul

- The event system has been completely redesigned from a C-style union to a **type-safe tagged variant**.

- `pollEvent()` now returns `sf::base::Optional<sf::Event>` instead of taking an output parameter.

```cpp
//
// BEFORE (upstream SFML)
sf::Event event;
while (window.pollEvent(event))
{
    if (event.type == sf::Event::Closed)
        window.close();
    else if (event.type == sf::Event::KeyPressed)
        handleKey(event.key.code);
}

//
// AFTER (VRSFML)
while (const auto event = window.pollEvent())
{
    if (event->is<sf::Event::Closed>())
        return 0;
    else if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>())
        handleKey(keyPress->scancode);
}
```

- Each event type is a separate struct (e.g. `sf::Event::KeyPressed`, `sf::Event::MouseMoved`, etc.) with named fields.

- Three ways to check event types:
    - `event->is<sf::Event::Closed>()` -- boolean check.
    - `event->getIf<sf::Event::KeyPressed>()` -- returns pointer to data, or `nullptr`.
    - `event->visit(visitor)` -- full visitor pattern.

- **Bulk event handling** is supported via `pollAndHandleEvents`:

```cpp
window.pollAndHandleEvents(
    [&](sf::Event::Closed) { mustClose = true; },
    [&](const sf::Event::KeyPressed& e) { handleKey(e.scancode); },
    [&](const sf::Event::MouseMoved& e) { handleMouse(e.position); }
    // Unhandled event types are silently ignored.
);
```

- New event type: `sf::Event::MouseMovedRaw` provides unprocessed mouse delta input (no acceleration/smoothing), useful for camera control in 3D/first-person views.



## Window Styles Replaced By Booleans

- `sf::Style` bitfield flags have been replaced by individual boolean fields in `sf::WindowSettings`.

```cpp
//
// BEFORE (upstream SFML)
sf::RenderWindow window(videoMode, "Title", sf::Style::Titlebar | sf::Style::Close);

//
// AFTER (VRSFML)
auto window = sf::RenderWindow::create({.size{800u, 600u},
                                        .title = "Title",
                                        .resizable = false,
                                        .closable = true,
                                        .hasTitlebar = true}).value();
```

- Available boolean fields: `fullscreen`, `resizable`, `closable`, `hasTitlebar`.



## `sf::String` Replaced By `sf::UnicodeString`

- `sf::String` has been replaced by `sf::UnicodeString` with full UTF encoding support.

- Used throughout the API for window titles, clipboard text, keyboard descriptions, and joystick names.



## Angle Type

- Rotation values are now represented by the dedicated `sf::Angle` type instead of raw `float`.

```cpp
//
// BEFORE (upstream SFML)
sprite.setRotation(45.f);
float rot = sprite.getRotation();

//
// AFTER (VRSFML)
sprite.rotation = sf::degrees(45.f);
float rot = sprite.rotation.asDegrees();
```

- Factory functions: `sf::degrees(float)`, `sf::radians(float)`.
- Methods: `.asDegrees()`, `.asRadians()`, `.wrapSigned()`, `.wrapUnsigned()`.



## Clock Pause/Resume Support

- `sf::Clock` now supports pausing and resuming.

```cpp
sf::Clock clock;

clock.stop();               // Pause the clock
// ...
clock.start();              // Resume the clock

Time t1 = clock.restart();  // Reset and keep running
Time t2 = clock.reset();    // Reset and leave paused

bool running = clock.isRunning();
```

- `sf::Clock::now()` provides access to the raw monotonic clock without needing an instance.



## Color HSL Support

- `sf::Color` now supports HSL (Hue, Saturation, Lightness) color model conversion.

```cpp
// Create a color from HSL
sf::Color color = sf::Color::fromHSLA({.hue = 120.f, .saturation = 1.f, .lightness = 0.5f});

// Convert to HSL
sf::Color::HSL hsl = color.toHSL();

// Hue rotation
sf::Color rotated = color.withRotatedHue(90.f);

// Adjust saturation/lightness
sf::Color desaturated = color.withSaturation(0.2f);
sf::Color darker      = color.withLightness(0.3f);
```

- Convenience factories: `sf::Color::whiteWithAlpha(alpha)`, `sf::Color::blackWithAlpha(alpha)`.
- Packed integer: `sf::Color::fromRGBA(0xFF0000FF)` for opaque red.



## Font And Text API Changes

- `sf::Font` uses `openFromFile` (not `loadFromFile`) and returns `sf::base::Optional<sf::Font>`.
    - Fonts optionally accept a `sf::TextureAtlas*` for shared atlas packing:

```cpp
//
// BEFORE (upstream SFML)
sf::Font font;
font.loadFromFile("font.ttf");
sf::Text text(font, "Hello", 30);
text.setFillColor(sf::Color::Red);

//
// AFTER (VRSFML)
auto font = sf::Font::openFromFile("font.ttf").value();
sf::Text text(font, {.string = "Hello",
                      .characterSize = 30u,
                      .fillColor = sf::Color::Red});
```

- Text styling uses individual booleans instead of a style enum:

```cpp
//
// BEFORE (upstream SFML)
text.setStyle(sf::Text::Bold | sf::Text::Italic);

//
// AFTER (VRSFML)
sf::Text text(font, {.string = "Hello",
                      .bold = true,
                      .italic = true});
```

- Fonts are **not copyable** -- only movable.

- Construction of `sf::Text` from a temporary font is a **deleted overload** to prevent dangling references.



## New Shape Types

- VRSFML adds several new shape types beyond the original circle, rectangle, and convex shapes:

| Shape                         | Data Struct                    |
|-------------------------------|--------------------------------|
| Ellipse                       | `sf::EllipseShapeData`         |
| Ring (annulus)                | `sf::RingShapeData`            |
| Pie slice                     | `sf::PieSliceShapeData`        |
| Ring pie slice                | `sf::RingPieSliceShapeData`    |
| Rounded rectangle             | `sf::RoundedRectangleShapeData`|
| Star                          | `sf::StarShapeData`            |
| Arrow                         | `sf::ArrowShapeData`           |
| Curved arrow                  | `sf::CurvedArrowShapeData`     |

- All shapes follow the designated-initializer aggregate pattern for construction.



## Keyboard And Mouse API Changes

- `sf::Keyboard::Scancode` has been greatly expanded with 200+ scan codes from SDL3.

- New utility functions:
    - `sf::Keyboard::localize(Scancode)` -- convert scancode to virtual key.
    - `sf::Keyboard::delocalize(Key)` -- convert virtual key to scancode.
    - `sf::Keyboard::getDescription(Scancode)` -- human-readable key name.
    - `sf::Keyboard::setVirtualKeyboardVisible(bool)` -- for mobile/touch platforms.

- Touch input now provides device enumeration via `sf::Touch::getDevices()` and `sf::Touch::Device` structs.



## Lifetime Tracking

- VRSFML has an optional compile-time lifetime tracking system (`SFML_ENABLE_LIFETIME_TRACKING`) that catches dangling references in debug builds.

- For example, constructing an `sf::Text` from a temporary `sf::Font` is a compile error (deleted overload). At runtime, if a `SoundBuffer` is destroyed while a `Sound` still references it, the lifetime tracker will assert.

- This prevents the "white square problem" from upstream SFML where a sprite would silently render incorrectly after its texture was destroyed.
