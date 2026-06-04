# Migration Guide

Welcome to SFML 3!
The SFML Team has put a lot of effort into delivering a library that is both familiar to existing users while also making significant improvements.
This document will walk you through how to upgrade your SFML 2 application to SFML 3.

One of the headline changes in SFML 3 is raising the C++ standard to C++17 thus bringing SFML into the world of modern C++!
This change has enabled a vast number of internal improvements as well as new opportunities for improving the API that will be discussed in this document.

## Compiler Requirements

Along with the upgrade from C++03 to C++17 you may need to upgrade your compiler.
C++17 support has been widespread in all major compiler implementations for years prior to SFML 3's release so in all likelihood the compiler you're already using will work.
In case you do need to upgrade, here are the minimum compiler versions.

| Compiler   | Version      |
| ---------- | ------------ |
| MSVC       | 16 (VS 2019) |
| GCC        | 9            |
| Clang      | 9            |
| AppleClang | 12           |

## CMake Targets

SFML 3 uses modern CMake convention for library targets which entails having a namespace in front of the target name.
These namespaces tell CMake "this is a target" whereas something like `zancle-graphics` might be a target or might be a precompiled library on disk like `libsfml-graphics.so`.
Fixing this ambiguity leads to more useful error messages when a given target can't be found due to, for example, forgetting a `find_package` call.
The component names used when calling `find_package` were also changed to capitalize the first letter.

| v2 Target       | v3 Target        |
| --------------- | ---------------- |
| `zancle-system`   | `Zancle::System`   |
| `zancle-window`   | `Zancle::Window`   |
| `zancle-graphics` | `Zancle::Graphics` |
| `zancle-network`  | `Zancle::Network`  |
| `zancle-audio`    | `Zancle::Audio`    |
| `zancle-main`     | `Zancle::Main`     |

| v2 Component | v3 Component |
| ------------ | ------------ |
| `system`     | `System`     |
| `window`     | `Window`     |
| `graphics`   | `Graphics`   |
| `network`    | `Network`    |
| `audio`      | `Audio`      |
| `main`       | `Main`       |


v2:
```cmake
find_package(SFML 2 REQUIRED COMPONENTS graphics audio network)
...
target_link_libraries(my_app PRIVATE zancle-graphics zancle-audio zancle-network)
```

v3:
```cmake
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
...
target_link_libraries(my_app PRIVATE Zancle::Graphics Zancle::Audio Zancle::Network)
```

## Linux Dependencies

When using X11 as the backend on Linux, as opposed to DRM, `libxi-dev` is a newly required dependency.
This was introduced with the [raw mouse input support](https://www.zancle-dev.org/documentation/3.0.0/structsf_1_1Event_1_1MouseMovedRaw.html).

## `za::Vec2<T>` Parameters

A common pattern in SFML 2 was to use pairs of scalar parameters to represent concepts like sizes or positions.
Take `za::Transformable::setPosition(float, float)` for example.
The two parameters combine to represent a position in world space.

SFML 3 takes all of the APIs with pairs of parameters like `(float, float)` or `(unsigned int, unsigned int)` and converts them to their corresponding `za::Vec2<T>` type like `za::Vec2f` or `za::Vec2u` to make the interface more expressive and composable.
This transition is often as simple as wrapping the two adjacent parameters with braces to construct the vector.

v2:
```cpp
za::VideoMode videoMode(480, 640, 24);
za::CircleShape circle(10);
circle.position = 10, 20;
za::IntRect rect(250, 400, 50, 100);
```

v3:
```cpp
za::VideoMode videoMode({480, 640}, 24);
za::CircleShape circle(10);
circle.position = {10, 20};
za::IntRect rect({250, 400}, {50, 100});
```

## Fixed Width Integers

SFML 2 contained various typedefs for fixed width integers.
Those are now replaced with the fixed width integers provided in the `<cstdint>` header from the standard library.

| v2           | v3              |
| ------------ | --------------- |
| `za::Int8`   | `std::int8_t`   |
| `za::Uint8`  | `std::uint8_t`  |
| `za::Int16`  | `std::int16_t`  |
| `za::Uint16` | `std::uint16_t` |
| `za::Int32`  | `std::int32_t`  |
| `za::Uint32` | `std::uint32_t` |
| `za::Int64`  | `std::int64_t`  |
| `za::Uint64` | `std::uint64_t` |

## `za::Event`

SFML 3 uses `std::variant` under the hood to implement a totally new, type-safe API for events.
There are two main ways to use this new API.
Check out the new EventHandling example program to see these methods in practice.

### `za::Event::getIf<T>`

The first option is based around `za::Event::getIf<T>` and `za::Event::is<T>`.
`getIf<T>` works by providing a template parameter which must be an event subtype.
Event subtypes are types like `za::Event::Resized` or `za::Event::MouseMoved`.
If the template argument matches the active event subtype then a pointer to the data is returned.
If that template argument is not the active event subtype then `nullptr` is returned.
`za::Event::is<T>` more simply returns `true` if `T` matches the active event subtype.
`is<T>` is often used for subtypes like `za::Event::Closed` which contain no data.
Here's what that looks like:

```cpp
while (window.isOpen())
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<za::Event::Closed>())
        {
            window.close();
        }
        else if (const auto* keyPressed = event->getIf<za::Event::KeyPressed>())
        {
            if (keyPressed->scancode == za::Keyboard::Scancode::Escape)
                window.close();
        }
    }

    // Remainder of main loop
}
```

Note how the API for getting events has changed slightly.
`za::WindowBase::pollEvent` and `za::WindowBase::waitEvent` now return a `std::optional<za::Event>`.
These two functions _might_ return an event but they might not.
C++ lets you deduce the template parameter which is why you can write `const std::optional event` instead of `const std::optional<za::Event> event`.
`const auto event` is another valid choice if you prefer a more terse expression.

### `za::WindowBase::handleEvents`

The second option for processing events is via the new `za::WindowBase::handleEvents` function.
This functions performs event visitation.
What this means is that you can provide callbacks which take different event subtypes as arguments.
Alternatively you may provide an object (or objects) with `operator()` implementations which handle the event subtypes you want to process.
Notably you do not have to provide callbacks for all possible event subtypes.
Depending on the current active event subtype, the corresponding callback is called.
Here's what that looks like:

```cpp
const auto onClose = [&window](const za::Event::Closed&)
{
    window.close();
};

const auto onKeyPressed = [&window](const za::Event::KeyPressed& keyPressed)
{
    if (keyPressed.scancode == za::Keyboard::Scancode::Escape)
        window.close();
};

while (window.isOpen())
{
    window.handleEvents(onClose, onKeyPressed);

    // Remainder of main loop
}
```

## Window Styles and States

A new `za::State` enumeration was added for specifying the state of the window which means whether the window is floating or fullscreen.
Here's a before-and-after showing how this affects constructing a window.

v2:
```cpp
za::RenderWindow window(za::VideoMode::getFullscreenModes().at(0), "Title", za::Style::Fullscreen);
```

v3:
```cpp
za::RenderWindow window(za::VideoMode::getFullscreenModes().at(0), "Title", za::State::Fullscreen);
```

## Scoped Enumerations

SFML 3 converts all old style unscoped enumerations to scoped enumerations.
This improves the type safety of the interface.
This means that the name of the enumeration is now part of the namespace required to access values of that enumeration.

For example, take the enumeration `za::Keyboard::Key`.
`za::Keyboard::A` becomes `za::Keyboard::Key::A`.
The name of the enumeration now appears as a nested namespace when accessing one of the enumeration's values.

Here is a complete list of all enumerations which have undergone this change:

* `za::BlendMode::Equation`
* `za::BlendMode::Factor`
* `za::Cursor::Type`
* `za::Ftp::Response::Status`
* `za::Ftp::TransferMode`
* `za::Http::Request::Method`
* `za::Http::Response::Status`
* `za::Joystick::Axis`
* `za::Keyboard::Key`
* `za::Keyboard::Scan`
* `za::Mouse::Button`
* `za::Mouse::Wheel`
* `za::PrimitiveType`
* `za::Sensor::Type`
* `za::Shader::Type`
* `za::Socket::Status`
* `za::Socket::Type`
* `za::SoundSource::Status`
* `za::VertexBuffer::Usage`

## `za::Rect<T>`

`za::Rect<T>` has been refactored from the four scalar values `top`, `left`, `width`, and `height` into two `za::Vec2<T>`s named `position` and `size`.
This means that `za::Rect<T>::getPosition()` and `za::Rect<T>::getSize()` have been removed in favor of directly accessing the `position` and `size` data members.
The 4-parameter constructor was also removed in favor of the constructor which takes two `za::Vec2<T>`s.

| v2        | v3            |
| --------- | ------------- |
| `.left`   | `.position.x` |
| `.top`    | `.position.y` |
| `.width`  | `.size.x`     |
| `.height` | `.size.y`     |

v2:
```cpp
za::FloatRect rect(10, 20, 30, 40);
za::Vec2f position = rect.position;;
za::Vec2f size = rect.getSize();
```

v3:
```cpp
za::FloatRect rect({10, 20}, {30, 40});
za::Vec2f position = rect.position;
za::Vectro2f size = rect.size;
```

The two overloads of `za::Rect<T>::intersects` have been replaced with one unified function called `za::Rect<T>::findIntersection` which returns a `std::optional<Rect<T>>`.
This optional contains the overlapping area if the rectangles overlap.
Otherwise the optional is empty.

```cpp
za::IntRect rect1({0, 0}, {200, 200});
za::IntRect rect2({100, 100}, {200, 200});
std::optional<za::IntRect> intersection = rect1.findIntersection(rect2);
// position={100, 100} size={100, 100}
```

## `za::Angle`

All angles are now represented with a strong type named `za::Angle`.
This type provides two functions for creating angles called `za::degrees(float)` and `za::radians(float)` which construct an angle from either some value of degrees or radians.
Operators (`+`, `-`, etc.) are provided to perform mathematical operations with angles.
If you need access to the raw angle as a `float` then you can use either `za::Angle::asDegrees()` or `za::Angle::asRadians()`.

v2:
```cpp
za::RectangleShape shape(za::Vec2f(50, 50));
shape.rotation = 90;
std::cout << "Rotation: " << shape.rotation << '\n';
```

v3:
```cpp
za::RectangleShape shape({50, 50});
shape.rotation = za::degrees(90);
std::cout << "Rotation: " << shape.rotation.asDegrees() << '\n';
```

## Renamed Functions

A number of functions have new names but otherwise have not changed their semantics.

| v2                                | v3                                |
| --------------------------------- | --------------------------------- |
| `za::Font::loadFromFile`          | `za::Font::openFromFile`          |
| `za::Socket::getHandle`           | `za::Socket::getNativeHandle`     |
| `za::WindowBase::getSystemHandle` | `za::WindowBase::getNativeHandle` |
| `za::Texture::create`             | `za::Texture::resize`             |
| `za::RenderTexture::create`       | `za::RenderTexture::resize`       |
| `za::Image::create`               | `za::Image::resize`               |
| `za::Sound::getLoop`              | `za::Sound::isLooping`            |
| `za::Sound::setLoop`              | `za::Sound::setLooping`           |
| `za::SoundStream::getLoop`        | `za::SoundStream::isLooping`      |
| `za::SoundStream::setLoop`        | `za::SoundStream::setLooping`     |

## Removal of Default Constructors

The default constructors `za::Sound::Sound()`, `za::Text::Text()`, and `za::Sprite::Sprite()` were removed.
They can be replaced by the corresponding constructors which accept a resource type.

| v2                     | v3                                         |
| ---------------------- | ------------------------------------------ |
| `za::Sound::Sound()`   | `za::Sound::Sound(const za::SoundBuffer&)` |
| `za::Text::Text()`     | `za::Text::Text(const za::Font&)`          |
| `za::Sprite::Sprite()` | `za::Sprite::Sprite(const za::Texture&)`   |

Now that these classes are guaranteed to be holding a reference to their corresponding resource type, the functions used to access to those resources can return a reference instead of a pointer.
These functions are `za::Sound::getBuffer()`, `za::Text::getFont()`, and `za::Sprite::getTexture()`.

v2:
```cpp
const za::SoundBuffer soundBuffer("sound.flac");
za::Sound sound;
sound.setBuffer(soundBuffer);
```

v3:
```cpp
const za::SoundBuffer soundBuffer("sound.flac");
za::Sound sound(soundBuffer);
```

## `std::optional` Usage

SFML 3 makes liberal use of `std::optional` to express when a given function may not return a value.
Some of these usages have already been mentioned like `za::WindowBase::pollEvent`.
Here are some more places where SFML 3 makes use of `std::optional`.

* `za::IpAddress` uses `std::optional` to express how resolving an address from a string may not yield a usable IP address.
* `za::Image::saveToMemory` returns a `std::optional` because the `za::Image` may be empty or the underlying implementation may fail.
* `za::SoundFileReader::open` returns a `std::optional` because the stream being opened may not be valid.
* `za::Music::onLoop` and `za::SoundStream::onLoop` returns a `std::optional` because if the objects are not in a looping state then there is nothing to return.
* `za::InputStream` uses `std::optional` in various places.
Instead of returning `-1` to signal an error, `std::nullopt` can be returned.

LearnCpp.com is a great place to learn more about using `std::optional`.
Read more about that [here](https://www.learncpp.com/cpp-tutorial/stdoptional/).

## Fixed Width Integers

SFML 2 contained various typedefs for fixed width integers.
Those are now replaced with the fixed width integers provided in the `<cstdint>` header from the standard library.

| v2           | v3              |
| ------------ | --------------- |
| `za::Int8`   | `base::I8`   |
| `za::Uint8`  | `base::U8`  |
| `za::Int16`  | `base::I16`  |
| `za::Uint16` | `base::U16` |
| `za::Int32`  | `base::I32`  |
| `za::Uint32` | `base::U32` |
| `za::Int64`  | `base::I64`  |
| `za::Uint64` | `base::U64` |

## New Constructors for Loading Resources

The following classes gained constructors that allow for loading/opening resources in a single expression.
Upon failure they throw an `za::Exception`.

* `za::InputSoundFile`
* `za::OutputSoundFile`
* `za::Music`
* `za::SoundBuffer`
* `za::Font`
* `za::Image`
* `za::RenderTexture`
* `za::Shader`
* `za::Texture`
* `za::FileInputStream`
* `za::Cursor`

SFML 3 still supports the SFML 2 style of error handling in addition to these new constructors.

v2:
```cpp
za::SoundBuffer soundBuffer;
if (!soundBuffer.loadFromFile("sound.wav"))
{
    // Handle error
}
```

v3:
```cpp
za::SoundBuffer soundBuffer;
if (!soundBuffer.loadFromFile("sound.wav"))
{
    // Handle error
}

// OR

const za::SoundBuffer soundBuffer("sound.wav");
```

## `za::Vec2<T>` and `za::Vec3<T>` Utility Functions

`za::Vec2<T>` and `za::Vec3<T>` gained a number of new functions for performing common mathematic operations on vectors.

| `za::Vec2<T>` Function       | Description                              |
| ------------------------------- | ---------------------------------------- |
| `Vec2(T, za::Angle)`         | Construct from polar coordinates         |
| `length()`                      | Get length                               |
| `lengthSquared()`               | Get length squared                       |
| `normalized()`                  | Get vector normalized to unit circle     |
| `angleTo(za::Vec2)`          | Get angle to another vector              |
| `angle()`                       | Get angle from X axis                    |
| `rotatedBy(za::Angle)`          | Get vector rotated by a given angle      |
| `projectedOnto(za::Vec2)`    | Get vector projected onto another vector |
| `perpendicular()`               | Get perpendicular vector                 |
| `dot(za::Vec2)`              | Get dot product                          |
| `cross(za::Vec2)`            | Get Z component of cross product         |
| `componentWiseMul(za::Vec2)` | Get component-wise multiple              |
| `componentWiseDiv(za::Vec2)` | Get component-wise divisor               |

| `za::Vec3<T>` Function       | Description                           |
| ------------------------------- | ------------------------------------- |
| `length()`                      | Get length                            |
| `lengthSquared()`               | Get length squared                    |
| `normalized()`                  | Get vector normalized to unit circle  |
| `dot(za::Vec3)`              | Get dot product                       |
| `cross(za::Vec3)`            | Get cross product                     |
| `componentWiseMul(za::Vec3)` | Get component-wise multiple           |
| `componentWiseDiv(za::Vec3)` | Get component-wise divisor            |

## Threading Primitives

`za::Lock`, `za::Mutex`, `za::Thread`, `za::ThreadLocal`, and `za::ThreadLocalPtr` were removed and replaced with their equivalents from the standard library.
The standard library provides multiple options for threads, locks, and mutexes among other threading primitives.

| v2                   | v3                                               |
| -------------------- | ------------------------------------------------ |
| `za::Lock `          | `std::lock_guard` or `std::unique_lock`          |
| `za::Mutex`          | `std::mutex` or `std::recursive_mutex`           |
| `za::Thread`         | `std::thread` or `std::jthread` (requires C++20) |
| `za::ThreadLocal`    | `thread_local`                                   |
| `za::ThreadLocalPtr` | `thread_local`                                   |

## Sound Samples and Channel Map

SFML 3 introduces the concept of a Channel Map which defines the mapping of the position in sample frame to the sound channel.
For example, if you have a sound frame with six different samples for a 5.1 sound system, the Channel Map defines how each of those samples map to which speaker channel.

SFML 2 always assumed the order as specified by OpenAL.

```cpp
auto samples = std::vector<std::int16_t>();
// ...

auto channelMap = std::vector<za::SoundChannel>{
    za::SoundChannel::FrontLeft,
    za::SoundChannel::FrontCenter,
    za::SoundChannel::FrontRight,
    za::SoundChannel::BackRight,
    za::SoundChannel::BackLeft,
    za::SoundChannel::LowFrequencyEffects
};
auto soundBuffer = za::SoundBuffer(samples.data(), samples.size(), channelMap.size(), 44100, channelMap);
auto sound = za::Sound(soundBuffer);
```

This a breaking change for the following APIs:

- `bool za::SoundBuffer::loadFromSamples(...)`
- `bool za::SoundBuffer::update(...)`
- `void za::SoundStream::initialize(...)`
- `bool za::OutputSoundFile::openFromFile(...)`
- `bool za::SoundFileWriter::open(...)`

## Deprecated APIs

SFML 3 removes all of the deprecated APIs in SFML 2.

| Deprecated API                      | Replacement                        |
| ----------------------------------- | ---------------------------------- |
| `za::Event::MouseWheelEvent`        | `za::Event::MouseWheelScrolled`    |
| `za::RenderWindow::capture`         | See 1                              |
| `za::RenderTexture::create`         | `za::RenderTexture::resize`        |
| `za::Shader::setParameter`          | `za::Shader::setUniform`           |
| `za::Text::setColor`                | `za::Text::setFillColor`           |
| `za::Text::getColor`                | `za::Text::getFillColor`           |
| `za::PrimitiveType::LinesStrip`     | `za::PrimitiveType::LineStrip`     |
| `za::PrimitiveType::TrianglesStrip` | `za::PrimitiveType::TriangleStrip` |
| `za::PrimitiveType::TrianglesFan`   | `za::PrimitiveType::TriangleFan`   |
| `za::PrimitiveType::Quads`          | See 2                              |
| `za::Keyboard::BackSlash`           | `za::Keyboard::Key::Backslash`     |
| `za::Keyboard::BackSpace`           | `za::Keyboard::Key::Backspace`     |
| `za::Keyboard::Dash`                | `za::Keyboard::Key::Dash`          |
| `za::Keyboard::Quote`               | `za::Keyboard::Key::Hyphen`        |
| `za::Keyboard::Return`              | `za::Keyboard::Key::Enter`         |
| `za::Keyboard::SemiColon`           | `za::Keyboard::Key::Semicolon`     |
| `za::Keyboard::Tilde`               | `za::Keyboard::Key::Grave`         |

1. `za::RenderWindow::capture` can be recreated by using an `za::Texture` and its `za::Texture::update(const Window&)` function to copy its contents into an `za::Image` instead.
2. `za::PrimitiveType::Quads` can be replaced by another primitive type.
This is not a drop-in replacement but rather will require refactoring your code to work with a new geometry.
One viable option is to use `za::PrimitiveType::Triangles` where two adjacent triangles join to form what was previously one quad.

## Anti-Aliasing Renamed

SFML 3 capitalizes the `A` of `aliasing` for all the APIs.

* `za::RenderTexture::getMaximumAntialiasingLevel()` becomes `za::RenderTexture::getMaximumAntiAliasingLevel()`
* `za::ContextSettings::antiAliasingLevel` becomes `za::ContextSettings::antiAliasingLevel`

## CoordinateType for RenderStates

The enum `za::CoordinateType` was moved from the `za::Texture` into its own dedicated enum class.

The `za::RenderStates` class got a new member of type `za::CoordinateType` to control how the texture coordinates will be interpreted.
By default SFML uses `za::CoordinateType::Pixels`, while `za::CoordinateType::Normalized` is the default for OpenGL.
Using `za::CoordinateType::Normalized` with `za::RenderStates` allows for using normalized textures with `za::VertexArray` and `za::VertexBuffer`.

Additionally, as SFML now supports Stencil Testing, there's an additional overload for the Stencil Mode.

The constructor for `za::RenderStates` has changed.

v2:
```cpp
auto renderStates = za::RenderStates(za::BlendAlpha,
                                     transform,
                                     texture,
                                     nullptr);
```

v3:
```cpp
auto renderStates = za::RenderStates(za::BlendAlpha,
                                     za::StencilMode(),
                                     transform,
                                     za::CoordinateTye::Pixels,
                                     texture,
                                     nullptr);
```

## Other Minor Changes

SFML 3 includes various smaller changes that ought to be mentioned.

* Changed the parameter order of the `za::Text` constructor, so that the provided font is always the first parameter
* Reverted to default value of CMake's `BUILD_SHARED_LIBS` which means SFML now builds static libraries by default
* Changed `za::String` interface to use `std::u16string` and `std::u32string`
* Removed `za::ContextSettings` constructor in favor of aggregate initialization
* Removed `za::View::reset` in favor of assigning from a new `za::View` object
* Removed `za::Vertex` constructors in favor of aggregate initialization
* Renamed `za::Mouse::Button::XButton1` and `za::Mouse::Button::XButton2` enumerators to `za::Mouse::Button::Extra1` and `za::Mouse::Button::Extra2`
* Removed NonCopyable.hpp header in favor of using built-in language features for disabling copy operators
* Converted the following classes to namespaces: `za::Clipboard`, `za::Keyboard`, `za::Joystick`, `za::Listener`, `za::Mouse`, `za::Sensor`, `za::Touch`, `za::Vulkan`
* Removed `za::SoundStream::setProcessingInterval` as miniaudio matches the internal processing rate to the underlying backend
