<!-- TODO: WindowSettings -->

# Differences between upstream SFML and Vittorio's fork

## User-facing changes

- ➡️ **Built-in Emscripten support:**
    - This fork now works out-of-the-box with [Emscripten](https://emscripten.org/).
    - All existing examples and tests run flawlessly in the browser.
    - No explicit `#ifdef SFML_SYSTEM_EMSCRIPTEN` is required anywhere in user code.

<br>

- ➡️ **Built-in ImGui support:**
    - Adds a new `SFML::ImGui` module that depends on `SFML::Graphics`.
    - Can be controlled via the CMake option `SFML_BUILD_IMGUI`.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    sf::GraphicsContext     graphicsContext;               // Holds all "global" OpenGL state
    sf::ImGui::ImGuiContext imGuiContext(graphicsContext); // Holds all "global" ImGui state

    sf::RenderWindow window(graphicsContext, {.size{640u, 480u}, .title = "ImGui + SFML = <3"});
    sf::Clock deltaClock;

    while (true)
    {
        // `sf::base::Optional` is a drop-in replacement for `std::optional`
        while (const sf::base::Optional event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        // Updates the ImGui state and sets `window` as the active ImGui window
        imGuiContext.update(window, deltaClock.restart());

        // Native ImGui functions can be used after `imGuiContext.update`
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();
        imGuiContext.render(window);
        window.display();
    }
    ```

    </details>

<br>

- ➡️ **Complete removal of legacy OpenGL:**
    - Upstream SFML still uses legacy OpenGL calls such as `glBegin` and `glEnd`.
    - This fork now internally uses modern OpenGL that is compatible with OpenGL 3.0 ES.
    - OpenGL ES 3.0 is now supported on all platforms, including Windows via ANGLE.
    - This fork provides built-in shaders that use the following API:
        ```glsl
        uniform mat4 sf_u_textureMatrix;
        uniform mat4 sf_u_modelViewProjectionMatrix;

        in vec2 sf_a_position;
        in vec4 sf_a_color;
        in vec2 sf_a_texCoord;

        out vec4 sf_v_color;
        out vec2 sf_v_texCoord;
        ```

<br>

- ➡️ **Support for simultaneous audio devices:**
    - Upstream SFML does not support simulataneous different audio devices -- only one playback device and one capture device can be active at any time.
    - This fork supports multiple different audio devices at the same time (see `multi_audio_device` example): this allows, for example, for game sounds to be played on speakers while multiplayer VOIP is played on headphones.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    // Create sound sources
    auto music0 = sf::Music::openFromFile("resources/doodle_pop.ogg").value();
    auto music1 = sf::Music::openFromFile("resources/ding.flac").value();
    auto music2 = sf::Music::openFromFile("resources/ding.mp3").value();

    // Store all source sources together for convenience
    sf::SoundSource* const sources[]{&music0, &music1, &music2};

    // Create the audio context
    auto audioContext = sf::AudioContext::create().value();

    // For each hardware playback device, create a SFML playback device
    std::vector<sf::PlaybackDevice> playbackDevices;
    for (const sf::PlaybackDeviceHandle& deviceHandle :
         sf::AudioContextUtils::getAvailablePlaybackDeviceHandles(audioContext))
        playbackDevices.emplace_back(audioContext, deviceHandle);

    // Play multiple sources simultaneously on separate playback devices
    for (base::SizeT i = 0u; i < playbackDevices.size(); ++i)
        sources[i % 3]->play(playbackDevices[i]);
    ```

    </details>

<br>

- ➡️ **Restore factory-based creation APIs for SFML resources:**
    - Factory-based creation APIs considerably increased type-safety and usability of SFML resources as they completely eliminated the presence of an "empty state" and made it obvious to users where errors could occur, forcing them to decide between handling them, ignoring them, or propagating them.
    - Despite many months of work and discussion on factory-based APIs, factory-based have been reverted into legacy APIs with <https://github.com/SFML/SFML/pull/3152>, due to fear that users would find the migration from 2.x to 3.x too difficult.
    - This fork does not include https://github.com/SFML/SFML/pull/3152 and moves forward with the vision of a safer and more pedagogically valuable version of SFML.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    // ERROR, does not compile -- SFML resources do not have a default "empty state".
    /* sf::SoundBuffer soundBuffer; */

    // OK, user explicitly chose to throw if the file loading fails
    const auto soundBuffer = sf::SoundBuffer::loadFromFile(resourcesDir() / "ball.wav").value();

    // OK, user explicitly chose return if loading fails
    const auto optSoundBuffer = sf::SoundBuffer::loadFromFile(resourcesDir() / "ball.wav");
    if (!optSoundBuffer.hasValue()) { return EXIT_FAILURE; }
    ```

    </details>

<br>

- ➡️ **Debug lifetime tracking for all SFML resources:**
    - Catches common lifetime mistakes between dependee types (e.g. `sf::Font`) and dependant types (e.g. `sf::Text`) at run-time, providing the user with a readable error message.
    - Enabled for debug builds by default, can be controlled via the CMake option `SFML_ENABLE_LIFETIME_TRACKING`.
    - Rejected for upstream inclusion in <https://github.com/SFML/SFML/pull/3097>.
    - When enabled: zero compilation time impact, negligible runtime performance impact.

    <details>

    <summary>📜 Error message example</summary>

    ```text
    FATAL ERROR: a texture object was destroyed while existing sprite objects depended on it.

    Please ensure that every texture object outlives all of the sprite objects associated with it,
    otherwise those sprites will try to access the memory of the destroyed texture,
    causing undefined behavior (e.g., crashes, segfaults, or unexpected run-time behavior).

    One of the ways this issue can occur is when a texture object is created as a local variable
    in a function and passed to a sprite object. When the function has finished executing, the
    local texture object will be destroyed, and the sprite object associated with it will now be
    referring to invalid memory. Example:

        sf::Sprite createSprite()
        {
            sf::Texture texture(/* ... */);
            sf::Sprite sprite(texture, /* ... */);

            return sprite;
            //     ^~~~~~

            // ERROR: `texture` will be destroyed right after
            //        `sprite` is returned from the function!
        }

    Another possible cause of this error is storing both a texture and a sprite together in a
    data structure (e.g., `class`, `struct`, container, pair, etc...), and then moving that
    data structure (i.e., returning it from a function, or using `std::move`) -- the internal
    references between the texture and sprite will not be updated, resulting in the same
    lifetime issue.

    In general, make sure that all your texture objects are destroyed *after* all the
    sprite objects depending on them to avoid these sort of issues.
    ```

    </details>

<br>

- ➡️ **Compile-time--enforced lifetime correctness for `sf::Texture` and its dependants (`sf::Sprite` and `sf::Shape`):**
    - Rather than `sf::Sprite` and `sf::Shape` storing a `sf::Texture*` internally, which can easily become invalidated, the `sf::Texture*` is now passed at the point where it is required: the `sf::RenderTarget::draw` call.
    - This prevents common lifetime issues that SFML users have frequently encountered (i.e. "the white square problem") at compile-time, without any extra overhead.
    - This also promotes better code organization and a more linear lifetime hierarchy tree for our users.
    - This was proposed for upstream SFML in two different forms (https://github.com/SFML/SFML/pull/3072 and https://github.com/SFML/SFML/pull/3080), but rejected.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    sf::GraphicsContext graphicsContext;
    const auto texture = sf::Texture::loadFromFile(graphicsContext, "image.png").value();

    // ERROR, does not compile -- sprites do not store a texture pointer anymore.
    /* sf::Sprite sprite(texture); */

    // OK, prepare the sprite to eventually display the entire texture
    sf::Sprite sprite(texture.getRect());

    // ERROR, does not compile -- a texture (or the lack thereof) has to be provided during the draw call.
    /* window.draw(sprite); */

    // OK, user has a valid texture available at the point of the draw call -- no lifetime woes!
    window.draw(sprite, texture);
    ```

    </details>

<br>

- ➡️ **Removal of polymorphic inheritance trees:**
    - `sf::Drawable`, `sf::Shape`, and `sf::Transformable` have either been removed or made non-polymorphic.
    - These inheritance trees promote overuse of OOP and dynamic allocation, and move users away from data-oriented design.
        - In practice, it's not useful to have something like `std::vector<std::unique_ptr<sf::Drawable>>`, and it actually leads newcomers to poor software engineering practices.
        - If that sort of polymorphism is required in rare cases, it can always be obtained via `std::function` or other basic type erasure techniques.

<br>

- ➡️ **Removal of `sf::VertexArray` in lieu of `std::vector<sf::Vertex>`:**
    - `sf::VertexArray` was just a wrapper over `std::vector<sf::Vertex>` that exposes a subset of `std::vector`'s API.
    - `std::vector<sf::Vertex>` should be used instead, and users should be encouraged to do the same.
    - This was proposed for upstream SFML in <https://github.com/SFML/SFML/pull/3118>, but rejected.

<br>

- ➡️ **Removal of global state whenever possible:**
    - Upstream SFML is full of hidden global state: for example, any graphical resource or audio resource ends up interacting with a global registry (via `std::shared_ptr` and other expensive operations) on construction/destruction.
    - This fork removes any such hidden global state, and requires the user to decide where these registries live via `sf::AudioContext` and `sf::GraphicsContext`.
        - Generally, these context objects can be created at the beginning of `main` and passed downwards to the rest of the application.
    - Not only this change increases run-time performance and decreases compilation time overhead, but it also simplifies the internal implementation of SFML reducing the risk of subtle global initializiation fiasco issues and promoting users to write simpler software with a clear hierarchical lifetime structure.

<br>

- ➡️ **New `SFML::Base` module:**
    - New module containing abstractions and utilities generally useful in any C++ project.
    - Significantly decreases reliance on the Standard Library, providing drop-in replacements that are much faster to compile and much more performant at run-time even with optimizations disabled.
    - All components of `SFML::Base` have been carefully crafted to maximize compile-time thoughput, debug performance, and ease of use.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    // Drop-in replacement for `std::unique_ptr`:
    sf::base::UniquePtr<T> uPtr = sf::base::makeUnique<T>(/* ...`T` constructor args... */);

    // Fast PImpl idiom (zero allocation):
    sf::base::InPlacePImpl<T, 128 /* buffer size */> pImpl(/* ...`T` constructor args... */);

    // Fast traits (zero instantiation via compiler built-ins, does not virally include expensive `<type_traits>`)
    static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(T));

    // Fast math (uses compiler built-in if available)
    constexpr auto result = sf::base::cos(3.14f);

    // Fast index sequences (uses compiler built-in if available)
    constexpr auto indexSequence = SFML_BASE_MAKE_INDEX_SEQUENCE(32);

    // ...and much more...
    ```

    </details>

<br>

- ➡️ **`sf::Window` closed state has been removed:**
    - Windows are now considered always "open".
    - If a window needs to be closed/re-opened multiple times, it can be wrapped into an optional.
    - As shown by the examples, this makes code simpler and removes another unnecessary "empty state".

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    int main()
    {
        sf::GraphicsContext graphicsContext;
        sf::RenderWindow window(graphicsContext, screen, "Example window");

        while (true) // `window.isOpen()` does not exist anymore
        {
            while (const sf::base::Optional event = window.pollEvent())
                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return EXIT_SUCCESS; // `window.close()` does not exist anymore, just return
        }
    }

    // Need an empty state? Use `sf::base::Optional<sf::RenderWindow>`.
    ```

    </details>

<br>

- ➡️ **`sf::Socket` constructor now takes a `isBlocking` parameter:**
    - Following the principle of being more explicit, users now have to explicitly decide whether they want their socket to be blocking or not on construction, rather than relying on the possibly wrong default of blocking mode.

    <details>

    <summary>📜 Code example</summary>

    ```cpp
    // ERROR, does not compile -- blocking behavior not specified
    /* sf::UdpSocket socket; */

    // OK, blocking behavior explicitly provided
    sf::UdpSocket socket(/* isBlocking */ true);
    ```

    </details>

<br>

- ➡️ **Removal of catch-all module-wide headers like `Audio.hpp` and `Window.hpp`:**
    - These headers go against the principles of header hygiene, they promote poor practices and slow down users' projects compilation times.

<br>

- ➡️ **Simplified and polished examples:**
    - All examples have been manually reviewed and polished to be as idiomatic and simple as possible, reducing needless use of inheritance/polymorphism and removing unnecessary layers of abstraction.

## Implementation changes

- ➡️ **Changed C++ Standard to C++20:**
    - Some features (e.g. designated initializers. `[[likely]]`, `char8_t`, `constinit`, aggregate initialization using parentheses, concepts) are now used throughout the library.

<br>

- ➡️ **External dependencies are now downloaded and built:**
    - This work has been done by @binary1248 and will hopefully be merged into upstream SFML soon: <https://github.com/SFML/SFML/pull/3141>.

<br>

- ➡️ **Stack trace generation for errors and assertions:**
    - Human-readable stack traces are generated on any `sf::priv::err()` error message, assertion failure, or lifetime tracking error.
    - Internally uses `cpptrace`: <https://github.com/jeremy-rifkin/cpptrace>.
    - Can be controlled via the CMake option `SFML_ENABLE_STACK_TRACES`.

<br>

- ➡️ **Changed testing framework from Catch to Doctest:**
    - Doctest used to be upstream SFML's testing framework as per my proposal.
    - Doctest was changed to Catch2 in this PR <https://github.com/SFML/SFML/pull/2452> despite my objections.
    - Doctest has almost feature-parity with Catch2 but an insanely better compilation time impact: <https://github.com/doctest/doctest/>.

<br>

- ➡️ **Massive compilation time speedup:**
    - Thanks to copious use of PImpl and zero-allocation fast PImpl idioms, header hygiene, use of `SFML::Base` instead of the Standard Library, `extern template`, and many other techniques, this fork now compiles blazingly-fast compared to upstream SFML.

<br>

- ➡️ **`sf::priv::err()` enhancements:**
    - Including `Err.h` does not expose any expensive `ios` or `iostream` Standard Library header.
    - The end of a chain of streams is detected automatically, and a flush + newline is added at the end -- no need for `std::endl`!
    - Stack trace support, see above.

<br>

- ➡️ **Other various improvements:**
    - Optimize `sf::Shader` source loading performance by reading into thread-local vector.
    - Optimize rendering of `sf::Text` with outlines: now takes a single draw call compared to upstream SFML's two draw calls.
    - `sf::priv::Err` is now thread-safe.
    - All factory functions have been improved to support RVO or NRVO, checked via GCC's `-Wnrvo` flag.
    - Added `Vector2<T>::movedTowards(T r, Angle phi)` function.
    - `sf::Vector2`, `sf::Vector3`, and `sf::Rect` are now aggregates.
    - Removed catch-all headers such as `SFML/Audio.hpp` to promote good header hygiene in user projects.


<!--

## TODO

- `SFML_OS_EMSCRIPTEN`
    - All examples and tests work

- `SFML_ENABLE_LIFETIME_TRACKING`
- `SFML_BUILD_IMGUI`
- `SFML_ENABLE_STACK_TRACES`
- Doctest
- C++20 support
- Download dependencies
- EGL support on Windows via ANGLE
- Restore factory-based API
- New `SFML::Base` module
- Removal of global state whenever possible
    - No more `sf::AudioResource` nor `sf::GlResource`
- Massive compilation time speedup
- Simplified and polished examples
- Windows cannot be closed anymore, `isOpen` removed
- Built-in shaders
- No more legacy GL whatsoever
- Safer APIs
    - Sockets take blocking mode in ctor
- Better `priv::err()`
- Simultaneous multi-device sound API
- Removal of `sf::Drawable`

- Removal of catch-all headers like `Audio.hpp`
- Widespread usage of fast PImpl
- Much faster debug run-time performance
- Ahead with upstream
- Remove `sf::VertexArray`
- sf::Sprite does not store sf::Texture*




Add explicit template instantiations for vectors and rect

Make `Transformable` non-polymorphic
Make `Shape` non-polymorphic





    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();



      // Create the graphics context
    sf::GraphicsContext graphicsContext;

    // Create the window of the application with a stencil buffer
    sf::RenderWindow window(graphicsContext,

const auto font = sf::Font::openFromFile(graphicsContext, resourcesDir() / "tuffy.ttf").value();



        window.draw(leftPaddle, /* texture */ nullptr);
        window.draw(rightPaddle, /* texture */ nullptr);

-->
