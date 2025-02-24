////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/ConvexShape.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioContextUtils.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/SoundStream.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vector2.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdlib>


namespace
{
constexpr auto windowWidth  = 800u;
constexpr auto windowHeight = 600u;
constexpr auto pi           = 3.14159265359f;
constexpr auto sqrt2        = 2.f * 0.707106781186547524401f;

sf::Path resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources";
#endif
}


////////////////////////////////////////////////////////////
// Base class for effects
////////////////////////////////////////////////////////////
class Effect
{
public:
    virtual ~Effect() = default;

    [[nodiscard]] const std::string& getName() const
    {
        return m_name;
    }

    virtual void update(float time, float x, float y)                          = 0;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

    virtual void start(sf::PlaybackDevice&) = 0;
    virtual void stop()                     = 0;

    virtual void handleKey([[maybe_unused]] sf::Keyboard::Key key)
    {
    }

protected:
    explicit Effect(std::string name) : m_name(std::move(name))
    {
    }

private:
    std::string m_name;
};


////////////////////////////////////////////////////////////
// Surround Sound / Positional Audio Effect / Attenuation
////////////////////////////////////////////////////////////
class Surround : public Effect
{
public:
    explicit Surround(sf::Listener& listener, sf::Music&& music) :
    Effect("Surround / Attenuation"),
    m_listener(listener),
    m_music(std::move(music))
    {
        m_listenerShape.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f};

        // Set the music to loop
        m_music.setLooping(true);

        // Set attenuation to a nice value
        m_music.setAttenuation(0.04f);
    }

    void update(float /*time*/, float x, float y) override
    {
        m_position = {windowWidth * x - 10.f, windowHeight * y - 10.f};
        m_music.setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_listenerShape, /* texture */ nullptr, states);

        states.transform = sf::Transform::Identity;
        states.transform.translate(m_position);

        target.draw(m_soundShape, /* texture */ nullptr, states);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        m_music.play(playbackDevice);
    }

    void stop() override
    {
        m_music.stop();
    }

private:
    sf::Listener&   m_listener;
    sf::CircleShape m_listenerShape{{.fillColor = sf::Color::Red, .radius = 20.f}};
    sf::CircleShape m_soundShape{{.radius = 20.f}};
    sf::Vector2f    m_position;
    sf::Music       m_music;
};


////////////////////////////////////////////////////////////
// Pitch / Volume Effect
////////////////////////////////////////////////////////////
class PitchVolume : public Effect
{
public:
    explicit PitchVolume(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    Effect("Pitch / Volume"),
    m_listener(listener),
    m_pitchText(font, {.position = {windowWidth / 2.f - 120.f, windowHeight / 2.f - 80.f}}),
    m_volumeText(font, {.position = {windowWidth / 2.f - 120.f, windowHeight / 2.f - 30.f}}),
    m_music(std::move(music))
    {
        // Set the music to loop
        m_music.setLooping(true);

        // We don't care about attenuation in this effect
        m_music.setAttenuation(0.f);

        // Set initial pitch
        m_music.setPitch(m_pitch);

        // Set initial volume
        m_music.setVolume(m_volume);
    }

    void update(float /*time*/, float x, float y) override
    {
        m_pitch  = std::clamp(2.f * x, 0.f, 2.f);
        m_volume = std::clamp(100.f * (1.f - y), 0.f, 100.f);

        m_music.setPitch(m_pitch);
        m_music.setVolume(m_volume);

        m_pitchText.setString("Pitch: " + std::to_string(m_pitch));
        m_volumeText.setString("Volume: " + std::to_string(m_volume));
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_pitchText, states);
        target.draw(m_volumeText, states);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // We set the listener position back to the default
        // so that the music is right on top of the listener
        m_listener.position = {0.f, 0.f, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        m_music.play(playbackDevice);
    }

    void stop() override
    {
        m_music.stop();
    }

private:
    sf::Listener& m_listener;
    float         m_pitch{1.f};
    float         m_volume{100.f};
    sf::Text      m_pitchText;
    sf::Text      m_volumeText;
    sf::Music     m_music;
};


////////////////////////////////////////////////////////////
// Attenuation Effect
////////////////////////////////////////////////////////////
class Attenuation : public Effect
{
public:
    explicit Attenuation(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    Effect("Attenuation"),
    m_listener(listener),
    m_text(font,
           {.position = {20.f, 20.f},
            .string   = "Attenuation factor dampens full volume of sound while within inner cone based on distance to "
                        "listener.\nCone outer gain determines volume of sound while outside outer cone.\nWhen within "
                        "outer cone, volume is linearly interpolated between inner and outer volumes.",
            .characterSize = 18u}),
    m_music(std::move(music))
    {
        // Sound cone parameters
        static constexpr auto coneHeight     = windowHeight * 2.f;
        static constexpr auto outerConeAngle = sf::degrees(120.f);
        static constexpr auto innerConeAngle = sf::degrees(30.f);

        // Set common properties of both cones
        for (sf::ConvexShape* cone : {&m_soundConeOuter, &m_soundConeInner})
            cone->setPoint(0, {0.f, 0.f});

        // Make each cone based on their angle and height
        static constexpr auto makeCone = [](auto& shape, const auto& angle)
        {
            const auto theta = sf::degrees(90.f) - (angle / 2);
            const auto x     = coneHeight / std::tan(theta.asRadians());

            shape.setPoint(1, {-x, coneHeight});
            shape.setPoint(2, {x, coneHeight});
        };

        makeCone(m_soundConeOuter, outerConeAngle);
        makeCone(m_soundConeInner, innerConeAngle);

        // Set the music to loop
        m_music.setLooping(true);

        // Set attenuation factor
        m_music.setAttenuation(m_attenuation);

        // Set direction to face "downwards"
        m_music.setDirection({0.f, 1.f, 0.f});

        // Set cone
        m_music.setCone({innerConeAngle, outerConeAngle, 0.f});
    }

    void update(float /*time*/, float x, float y) override
    {
        m_position = {windowWidth * x - 10.f, windowHeight * y - 10.f};
        m_music.setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        auto statesCopy(states);

        statesCopy.transform = sf::Transform::Identity;
        statesCopy.transform.translate(m_position);

        target.draw(m_soundConeOuter, /* texture */ nullptr, statesCopy);
        target.draw(m_soundConeInner, /* texture */ nullptr, statesCopy);
        target.draw(m_soundShape, /* texture */ nullptr, statesCopy);
        target.draw(m_listenerShape, /* texture */ nullptr, states);
        target.draw(m_text, states);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        m_music.play(playbackDevice);
    }

    void stop() override
    {
        m_music.stop();
    }

private:
    sf::Listener&   m_listener;
    sf::CircleShape m_listenerShape{{.position  = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f + 100.f},
                                     .fillColor = sf::Color::Red,
                                     .radius    = 20.f}};
    sf::CircleShape m_soundShape{{.fillColor = sf::Color::Magenta, .radius = 20.f}};
    sf::ConvexShape m_soundConeOuter{{.position = {20.f, 20.f}, .fillColor = sf::Color::Black, .pointCount = 3u}};
    sf::ConvexShape m_soundConeInner{{.position = {20.f, 20.f}, .fillColor = sf::Color::Cyan, .pointCount = 3u}};
    sf::Text        m_text;
    sf::Vector2f    m_position;
    sf::Music       m_music;

    float m_attenuation{0.01f};
};


////////////////////////////////////////////////////////////
// Tone Generator
////////////////////////////////////////////////////////////
class Tone : public sf::SoundStream, public Effect
{
public:
    explicit Tone(sf::Listener& listener, const sf::Font& font) :
    Effect("Tone Generator"),
    m_listener(listener),
    m_instruction(font,
                  {.position = {windowWidth / 2.f - 370.f, windowHeight / 2.f - 200.f},
                   .string   = "Press up and down arrows to change the current wave type"}),
    m_currentType(font, {.position = {windowWidth / 2.f - 150.f, windowHeight / 2.f - 100.f}}),
    m_currentAmplitude(font, {.position = {windowWidth / 2.f - 150.f, windowHeight / 2.f - 50.f}}),
    m_currentFrequency(font, {.position = {windowWidth / 2.f - 150.f, windowHeight / 2.f}})
    {
        sf::SoundStream::initialize(1, sampleRate, {sf::SoundChannel::Mono});
    }

    void update(float /*time*/, float x, float y) override
    {
        m_amplitude = std::clamp(0.2f * (1.f - y), 0.f, 0.2f);
        m_frequency = std::clamp(500.f * x, 0.f, 500.f);

        m_currentAmplitude.setString("Amplitude: " + std::to_string(m_amplitude));
        m_currentFrequency.setString("Frequency: " + std::to_string(m_frequency) + " Hz");

        m_currentType.setString(std::string{"Wave Type: "} +
                                std::array{"Sine", "Square", "Triangle", "Sawtooth"}[static_cast<std::size_t>(m_type)]);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_instruction, states);
        target.draw(m_currentType, states);
        target.draw(m_currentAmplitude, states);
        target.draw(m_currentFrequency, states);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // We set the listener position back to the default
        // so that the tone is right on top of the listener
        m_listener.position = {0.f, 0.f, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        play(playbackDevice);
    }

    void stop() override
    {
        SoundStream::stop();
    }

    void handleKey(sf::Keyboard::Key key) override
    {
        if (key == sf::Keyboard::Key::Down)
            m_type = static_cast<Type>((static_cast<std::size_t>(m_type) + 1) % 4u); // Forward
        else if (key == sf::Keyboard::Key::Up)
            m_type = static_cast<Type>((static_cast<std::size_t>(m_type) - 1) % 4u); // Reverse
    }

private:
    bool onGetData(sf::SoundStream::Chunk& chunk) override
    {
        const auto period = 1.f / m_frequency;

        for (auto i = 0u; i < chunkSize; ++i)
        {
            auto value = 0.f;

            switch (m_type)
            {
                case Type::Sine:
                {
                    value = m_amplitude * std::sin(2 * pi * m_frequency * m_time);
                    break;
                }
                case Type::Square:
                {
                    value = m_amplitude *
                            (2 * (2 * std::floor(m_frequency * m_time) - std::floor(2 * m_frequency * m_time)) + 1);
                    break;
                }
                case Type::Triangle:
                {
                    value = 4 * m_amplitude / period *
                                std::abs(std::fmod(((std::fmod((m_time - period / 4), period)) + period), period) -
                                         period / 2) -
                            m_amplitude;
                    break;
                }
                case Type::Sawtooth:
                {
                    value = m_amplitude * 2 * (m_time / period - std::floor(0.5f + m_time / period));
                    break;
                }
            }

            m_sampleBuffer[i] = static_cast<std::int16_t>(std::lround(value * std::numeric_limits<std::int16_t>::max()));
            m_time += timePerSample;
        }

        chunk.sampleCount = chunkSize;
        chunk.samples     = m_sampleBuffer.data();

        return true;
    }

    void onSeek(sf::Time) override
    {
        // It doesn't make sense to seek in a tone generator
    }

    enum class Type : std::size_t
    {
        Sine,
        Square,
        Triangle,
        Sawtooth
    };

    static constexpr unsigned int sampleRate{44100};
    static constexpr std::size_t  chunkSize{sampleRate / 100};
    static constexpr float        timePerSample{1.f / float{sampleRate}};

    sf::Listener& m_listener;

    std::vector<std::int16_t> m_sampleBuffer = std::vector<std::int16_t>(chunkSize, 0);
    Type                      m_type{Type::Triangle};
    float                     m_amplitude{0.05f};
    float                     m_frequency{220};
    float                     m_time{};

    sf::Text m_instruction;
    sf::Text m_currentType;
    sf::Text m_currentAmplitude;
    sf::Text m_currentFrequency;
};


////////////////////////////////////////////////////////////
// Dopper Shift Effect
////////////////////////////////////////////////////////////
class Doppler : public sf::SoundStream, public Effect
{
public:
    explicit Doppler(sf::Listener& listener, const sf::Font& font) :
    Effect("Doppler Shift"),
    m_listener(listener),
    m_currentVelocity(font, {.position = {windowWidth / 2.f - 150.f, windowHeight * 3.f / 4.f - 50.f}}),
    m_currentFactor(font, {.position = {windowWidth / 2.f - 150.f, windowHeight * 3.f / 4.f}})
    {
        m_position.y = (windowHeight - 20.f) / 2.f - 40.f;

        // Set attenuation to a nice value
        setAttenuation(0.05f);

        sf::SoundStream::initialize(1, sampleRate, {sf::SoundChannel::Mono});
    }

    void update(float time, float x, float y) override
    {
        m_velocity = std::clamp(150.f * (1.f - y), 0.f, 150.f);
        m_factor   = std::clamp(x, 0.f, 1.f);

        m_currentVelocity.setString("Velocity: " + std::to_string(m_velocity));
        m_currentFactor.setString("Doppler Factor: " + std::to_string(m_factor));

        m_position.x = std::fmod(time, 8.f) * windowWidth / 8.f;

        setPosition({m_position.x, m_position.y, 0.f});
        setVelocity({m_velocity, 0.f, 0.f});
        setDopplerFactor(m_factor);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        auto statesCopy(states);
        statesCopy.transform = sf::Transform::Identity;
        statesCopy.transform.translate(m_position - sf::Vector2f({20.f, 0.f}));

        target.draw(m_listenerShape, /* texture */ nullptr, states);
        target.draw(m_soundShape, /* texture */ nullptr, statesCopy);
        target.draw(m_currentVelocity, states);
        target.draw(m_currentFactor, states);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        play(playbackDevice);
    }

    void stop() override
    {
        SoundStream::stop();
    }

private:
    bool onGetData(sf::SoundStream::Chunk& chunk) override
    {
        const auto period = 1.f / m_frequency;

        for (auto i = 0u; i < chunkSize; ++i)
        {
            const auto value = m_amplitude * 2 * (m_time / period - std::floor(0.5f + m_time / period));

            m_sampleBuffer[i] = static_cast<std::int16_t>(std::lround(value * std::numeric_limits<std::int16_t>::max()));
            m_time += timePerSample;
        }

        chunk.sampleCount = chunkSize;
        chunk.samples     = m_sampleBuffer.data();

        return true;
    }

    void onSeek(sf::Time) override
    {
        // It doesn't make sense to seek in a tone generator
    }

    static constexpr unsigned int sampleRate{44100};
    static constexpr std::size_t  chunkSize{sampleRate / 100};
    static constexpr float        timePerSample{1.f / float{sampleRate}};

    sf::Listener& m_listener;

    std::vector<std::int16_t> m_sampleBuffer = std::vector<std::int16_t>(chunkSize, 0);
    float                     m_amplitude{0.05f};
    float                     m_frequency{220};
    float                     m_time{};

    float           m_velocity{0.f};
    float           m_factor{1.f};
    sf::CircleShape m_listenerShape{
        {.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f}, .fillColor = sf::Color::Red, .radius = 20.f}};
    sf::CircleShape m_soundShape{{.radius = 20.f}};
    sf::Vector2f    m_position;
    sf::Text        m_currentVelocity;
    sf::Text        m_currentFactor;
};


////////////////////////////////////////////////////////////
// Processing base class
////////////////////////////////////////////////////////////
class Processing : public Effect
{
public:
    void update([[maybe_unused]] float time, float x, float y) override
    {
        m_position = {windowWidth * x - 10.f, windowHeight * y - 10.f};
        m_music.setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_listenerShape, /* texture */ nullptr, states);

        states.transform = sf::Transform::Identity;
        states.transform.translate(m_position);

        target.draw(m_soundShape, /* texture */ nullptr, states);
        target.draw(m_enabledText);
        target.draw(m_instructions);
    }

    void start(sf::PlaybackDevice& playbackDevice) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.updateListener(m_listener))
            std::cerr << "Failed to update listener\n";

        m_music.play(playbackDevice);
    }

    void stop() override
    {
        m_music.stop();
    }

protected:
    explicit Processing(sf::Listener& listener, const sf::Font& font, sf::Music&& music, std::string name) :
    Effect(std::move(name)),
    m_listener(listener),
    m_music(std::move(music)),
    m_enabledText(font, {.string = "Processing: Enabled"}),
    m_instructions(font, {.string = "Press Space to enable/disable processing"})
    {
        m_listenerShape.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f};

        m_enabledText.position  = {windowWidth / 2.f - 120.f, windowHeight * 3.f / 4.f - 50.f};
        m_instructions.position = {windowWidth / 2.f - 250.f, windowHeight * 3.f / 4.f};

        // Set the music to loop
        m_music.setLooping(true);

        // Set attenuation to a nice value
        m_music.setAttenuation(0.f);
    }


    sf::Listener& m_listener;

    // Needs to be defined above `m_music` because it's used in the music's effect processor
    bool m_enabled{true};

    sf::Music m_music;

private:
    void handleKey(sf::Keyboard::Key key) override
    {
        if (key == sf::Keyboard::Key::Space)
            m_enabled = !m_enabled;

        m_enabledText.setString(m_enabled ? "Processing: Enabled" : "Processing: Disabled");
    }

    sf::CircleShape m_listenerShape{{.fillColor = sf::Color::Red, .radius = 20.f}};
    sf::CircleShape m_soundShape{{.radius = 20.f}};
    sf::Vector2f    m_position;
    sf::Text        m_enabledText;
    sf::Text        m_instructions;
};


////////////////////////////////////////////////////////////
// Biquad Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
class BiquadFilter : public Processing
{
protected:
    struct Coefficients
    {
        float a0{};
        float a1{};
        float a2{};
        float b1{};
        float b2{};
        float c0{};
        float d0{};
    };

    using Processing::Processing;

    void setCoefficients(const Coefficients& coefficients)
    {
        struct State
        {
            float xnz1{};
            float xnz2{};
            float ynz1{};
            float ynz2{};
        };

        // We use a mutable lambda to tie the lifetime of the state and coefficients to the lambda itself
        // This is necessary since the Echo object will be destroyed before the music object
        // While the music object exists, it is possible that the audio engine will try to call
        // this lambda hence we need to always have usable coefficients and state until the music and the
        // associated lambda are destroyed
        m_music.setEffectProcessor(
            [coefficients, &enabled = m_enabled, state = std::vector<State>()](const float*  inputFrames,
                                                                               unsigned int& inputFrameCount,
                                                                               float*        outputFrames,
                                                                               unsigned int& outputFrameCount,
                                                                               unsigned int  frameChannelCount) mutable
            {
                // IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
                // will always be provided in frameChannelCount, this can be different from the channel count
                // of the audio source so make sure to size your buffers according to the engine and not the source
                // Ensure we have as many state objects as the audio engine has channels
                if (state.size() < frameChannelCount)
                    state.resize(frameChannelCount - state.size());

                for (auto frame = 0u; frame < outputFrameCount; ++frame)
                {
                    for (auto channel = 0u; channel < frameChannelCount; ++channel)
                    {
                        auto& channelState = state[channel];

                        const auto xn = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
                        const auto yn = coefficients.a0 * xn + coefficients.a1 * channelState.xnz1 +
                                        coefficients.a2 * channelState.xnz2 - coefficients.b1 * channelState.ynz1 -
                                        coefficients.b2 * channelState.ynz2;

                        channelState.xnz2 = channelState.xnz1;
                        channelState.xnz1 = xn;
                        channelState.ynz2 = channelState.ynz1;
                        channelState.ynz1 = yn;

                        outputFrames[channel] = enabled ? yn : xn;
                    }

                    inputFrames += (inputFrames ? frameChannelCount : 0u);
                    outputFrames += frameChannelCount;
                }

                // We processed data 1:1
                inputFrameCount = outputFrameCount;
            });
    }
};


////////////////////////////////////////////////////////////
// High-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct HighPassFilter : BiquadFilter
{
    explicit HighPassFilter(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    BiquadFilter(listener, font, std::move(music), "High-pass Filter")
    {
        static constexpr auto cutoffFrequency = 2000.f;

        const auto c = std::tan(pi * cutoffFrequency / static_cast<float>(m_music.getSampleRate()));

        Coefficients coefficients{.a0 = 1.f / (1.f + sqrt2 * c + std::pow(c, 2.f)),
                                  .a1 = -2.f * coefficients.a0,
                                  .a2 = coefficients.a0,
                                  .b1 = 2.f * coefficients.a0 * (std::pow(c, 2.f) - 1.f),
                                  .b2 = coefficients.a0 * (1.f - sqrt2 * c + std::pow(c, 2.f))};

        setCoefficients(coefficients);
    }
};


////////////////////////////////////////////////////////////
// Low-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct LowPassFilter : BiquadFilter
{
    explicit LowPassFilter(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    BiquadFilter(listener, font, std::move(music), "Low-pass Filter")
    {
        static constexpr auto cutoffFrequency = 500.f;

        const auto c = 1.f / std::tan(pi * cutoffFrequency / static_cast<float>(m_music.getSampleRate()));

        Coefficients coefficients{.a0 = 1.f / (1.f + sqrt2 * c + std::pow(c, 2.f)),
                                  .a1 = 2.f * coefficients.a0,
                                  .a2 = coefficients.a0,
                                  .b1 = 2.f * coefficients.a0 * (1.f - std::pow(c, 2.f)),
                                  .b2 = coefficients.a0 * (1.f - sqrt2 * c + std::pow(c, 2.f))};

        setCoefficients(coefficients);
    }
};


////////////////////////////////////////////////////////////
// Echo (miniaudio implementation)
////////////////////////////////////////////////////////////
struct Echo : Processing
{
    explicit Echo(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    Processing(listener, font, std::move(music), "Echo")
    {
        static constexpr auto delay = 0.2f;
        static constexpr auto decay = 0.75f;
        static constexpr auto wet   = 0.8f;
        static constexpr auto dry   = 1.f;

        const auto sampleRate    = m_music.getSampleRate();
        const auto delayInFrames = static_cast<unsigned int>(static_cast<float>(sampleRate) * delay);

        // We use a mutable lambda to tie the lifetime of the state to the lambda itself
        // This is necessary since the Echo object will be destroyed before the music object
        // While the music object exists, it is possible that the audio engine will try to call
        // this lambda hence we need to always have a usable state until the music and the
        // associated lambda are destroyed
        m_music.setEffectProcessor(
            [delayInFrames,
             &enabled = m_enabled,
             buffer   = std::vector<float>(),
             cursor   = 0u](const float*  inputFrames,
                          unsigned int& inputFrameCount,
                          float*        outputFrames,
                          unsigned int& outputFrameCount,
                          unsigned int  frameChannelCount) mutable
            {
                // IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
                // will always be provided in frameChannelCount, this can be different from the channel count
                // of the audio source so make sure to size your buffers according to the engine and not the source
                // Ensure we have enough space to store the delayed frames for all of the audio engine's channels
                if (buffer.size() < delayInFrames * frameChannelCount)
                    buffer.resize(delayInFrames * frameChannelCount - buffer.size(), 0.f);

                for (auto frame = 0u; frame < outputFrameCount; ++frame)
                {
                    for (auto channel = 0u; channel < frameChannelCount; ++channel)
                    {
                        const auto input = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
                        const auto bufferIndex = (cursor * frameChannelCount) + channel;
                        buffer[bufferIndex]    = (buffer[bufferIndex] * decay) + (input * dry);
                        outputFrames[channel]  = enabled ? buffer[bufferIndex] * wet : input;
                    }

                    cursor = (cursor + 1) % delayInFrames;

                    inputFrames += (inputFrames ? frameChannelCount : 0u);
                    outputFrames += frameChannelCount;
                }

                // We processed data 1:1
                inputFrameCount = outputFrameCount;
            });
    }
};


////////////////////////////////////////////////////////////
// Reverb (https://github.com/sellicott/DSP-FFMpeg-Reverb)
////////////////////////////////////////////////////////////
class Reverb : public Processing
{
public:
    explicit Reverb(sf::Listener& listener, const sf::Font& font, sf::Music&& music) :
    Processing(listener, font, std::move(music), "Reverb")
    {
        static constexpr auto sustain = 0.7f; // [0.f; 1.f]

        // We use a mutable lambda to tie the lifetime of the state to the lambda itself
        // This is necessary since the Echo object will be destroyed before the music object
        // While the music object exists, it is possible that the audio engine will try to call
        // this lambda hence we need to always have a usable state until the music and the
        // associated lambda are destroyed
        m_music.setEffectProcessor(
            [sampleRate = m_music.getSampleRate(),
             filters    = std::vector<ReverbFilter<float>>(),
             &enabled   = m_enabled](const float*  inputFrames,
                                   unsigned int& inputFrameCount,
                                   float*        outputFrames,
                                   unsigned int& outputFrameCount,
                                   unsigned int  frameChannelCount) mutable
            {
                // IMPORTANT: The channel count of the audio engine currently sourcing data from this sound
                // will always be provided in frameChannelCount, this can be different from the channel count
                // of the audio source so make sure to size your buffers according to the engine and not the source
                // Ensure we have as many filter objects as the audio engine has channels
                while (filters.size() < frameChannelCount)
                    filters.emplace_back(sampleRate, sustain);

                for (auto frame = 0u; frame < outputFrameCount; ++frame)
                {
                    for (auto channel = 0u; channel < frameChannelCount; ++channel)
                    {
                        const auto input = inputFrames ? inputFrames[channel] : 0.f; // Read silence if no input data available
                        outputFrames[channel] = enabled ? filters[channel](input) : input;
                    }

                    inputFrames += (inputFrames ? frameChannelCount : 0u);
                    outputFrames += frameChannelCount;
                }

                // We processed data 1:1
                inputFrameCount = outputFrameCount;
            });
    }

private:
    template <typename T>
    class AllPassFilter
    {
    public:
        AllPassFilter(std::size_t delay, float theGain) : m_buffer(delay, {}), m_gain(theGain)
        {
        }

        T operator()(T input)
        {
            const auto output  = m_buffer[m_cursor];
            input              = static_cast<T>(input + m_gain * output);
            m_buffer[m_cursor] = input;
            m_cursor           = (m_cursor + 1) % m_buffer.size();
            return static_cast<T>(-m_gain * input + output);
        }

    private:
        std::vector<T> m_buffer;
        std::size_t    m_cursor{};
        const float    m_gain{};
    };

    template <typename T>
    class FIRFilter
    {
    public:
        explicit FIRFilter(std::vector<float> taps) : m_taps(std::move(taps))
        {
        }

        T operator()(T input)
        {
            m_buffer[m_cursor] = input;
            m_cursor           = (m_cursor + 1) % m_buffer.size();

            T output{};

            for (auto i = 0u; i < m_taps.size(); ++i)
                output += static_cast<T>(m_taps[i] * m_buffer[(m_cursor + i) % m_buffer.size()]);

            return output;
        }

    private:
        const std::vector<float> m_taps;
        std::vector<T>           m_buffer = std::vector<T>(m_taps.size(), {});
        std::size_t              m_cursor{};
    };

    template <typename T>
    class ReverbFilter
    {
    public:
        ReverbFilter(unsigned int sampleRate, float feedbackGain) :
        m_allPass{{sampleRate / 10, 0.6f}, {sampleRate / 30, -0.6f}, {sampleRate / 90, 0.6f}, {sampleRate / 270, -0.6f}},
        m_fir({0.003369f,  0.002810f,  0.001758f,  0.000340f,  -0.001255f, -0.002793f, -0.004014f, -0.004659f,
               -0.004516f, -0.003464f, -0.001514f, 0.001148f,  0.004157f,  0.006986f,  0.009003f,  0.009571f,
               0.008173f,  0.004560f,  -0.001120f, -0.008222f, -0.015581f, -0.021579f, -0.024323f, -0.021933f,
               -0.012904f, 0.003500f,  0.026890f,  0.055537f,  0.086377f,  0.115331f,  0.137960f,  0.150407f,
               0.150407f,  0.137960f,  0.115331f,  0.086377f,  0.055537f,  0.026890f,  0.003500f,  -0.012904f,
               -0.021933f, -0.024323f, -0.021579f, -0.015581f, -0.008222f, -0.001120f, 0.004560f,  0.008173f,
               0.009571f,  0.009003f,  0.006986f,  0.004157f,  0.001148f,  -0.001514f, -0.003464f, -0.004516f,
               -0.004659f, -0.004014f, -0.002793f, -0.001255f, 0.000340f,  0.001758f,  0.002810f,  0.003369f}),
        m_buffer(sampleRate / 5, {}), // sample rate / 5 = 200ms buffer size
        m_feedbackGain(feedbackGain)
        {
        }

        T operator()(T input)
        {
            auto output = static_cast<T>(0.7f * input + m_feedbackGain * m_buffer[m_cursor]);

            for (auto& f : m_allPass)
                output = f(output);

            output = m_fir(output);

            m_buffer[m_cursor] = output;
            m_cursor           = (m_cursor + 1) % m_buffer.size();

            output += 0.5f * m_buffer[(m_cursor + 1 * m_interval - 1) % m_buffer.size()];
            output += 0.25f * m_buffer[(m_cursor + 2 * m_interval - 1) % m_buffer.size()];
            output += 0.125f * m_buffer[(m_cursor + 3 * m_interval - 1) % m_buffer.size()];

            return 0.6f * output + input;
        }

    private:
        AllPassFilter<T>  m_allPass[4];
        FIRFilter<T>      m_fir;
        std::vector<T>    m_buffer;
        std::size_t       m_cursor{};
        const std::size_t m_interval{m_buffer.size() / 3};
        const float       m_feedbackGain{};
    };
};

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Create the main window
    sf::RenderWindow window(
        {.size{windowWidth, windowHeight}, .title = "SFML Sound Effects", .resizable = false, .vsync = true});

    // Load the application font and pass it to the Effect class
    const auto font = sf::Font::openFromFile(resourcesDir() / "tuffy.ttf").value();

    // Exit early if music file not found
    const auto musicPath = resourcesDir() / "doodle_pop.ogg";
    if (!musicPath.exists())
    {
        std::cerr << "Music file '" << musicPath << "' not found, aborting" << std::endl;
        return EXIT_FAILURE;
    }

    // Create the playback device and listener
    // TODO P1: docs
    auto audioContext          = sf::AudioContext::create().value();
    auto playbackDeviceHandles = sf::AudioContextUtils::getAvailablePlaybackDeviceHandles(audioContext);

    std::size_t currentPlaybackDeviceIndex = 0;

    std::vector<sf::PlaybackDevice> playbackDevices;
    playbackDevices.reserve(playbackDeviceHandles.size());

    for (const sf::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        playbackDevices.emplace_back(audioContext, deviceHandle);

        if (deviceHandle.isDefault())
            currentPlaybackDeviceIndex = playbackDevices.size() - 1;
    }

    const auto getCurrentPlaybackDevice = [&]() -> sf::PlaybackDevice&
    { return playbackDevices.at(currentPlaybackDeviceIndex); };

    // TODO P1: docs
    sf::Listener listener;

    // Helper function to open a new instance of the music file
    const auto openMusic = [&] { return sf::Music::openFromFile(musicPath).value(); };

    // Create the effects
    Surround       surroundEffect(listener, openMusic());
    PitchVolume    pitchVolumeEffect(listener, font, openMusic());
    Attenuation    attenuationEffect(listener, font, openMusic());
    Tone           toneEffect(listener, font);
    Doppler        dopplerEffect(listener, font);
    HighPassFilter highPassFilterEffect(listener, font, openMusic());
    LowPassFilter  lowPassFilterEffect(listener, font, openMusic());
    Echo           echoEffect(listener, font, openMusic());
    Reverb         reverbEffect(listener, font, openMusic());

    const std::array<Effect*, 9> effects{&surroundEffect,
                                         &pitchVolumeEffect,
                                         &attenuationEffect,
                                         &toneEffect,
                                         &dopplerEffect,
                                         &highPassFilterEffect,
                                         &lowPassFilterEffect,
                                         &echoEffect,
                                         &reverbEffect};

    std::size_t current = 0;

    effects[current]->start(getCurrentPlaybackDevice());

    // Create the messages background
    const auto textBackgroundTexture = sf::Texture::loadFromFile(resourcesDir() / "text-background.png").value();

    // Create the description text
    sf::Text description(font,
                         {.position      = {10.f, 522.f},
                          .string        = "Current effect: " + effects[current]->getName(),
                          .characterSize = 20u,
                          .fillColor     = {80, 80, 80}});

    // Create the instructions text
    const sf::Text instructions(font,
                                {.position      = {280.f, 544.f},
                                 .string        = "Press left and right arrows to change the current effect",
                                 .characterSize = 20u,
                                 .fillColor     = {80, 80, 80}});

    // Utility functions
    const auto getCurrentDeviceName = [&] { return std::string{getCurrentPlaybackDevice().getDeviceHandle().getName()}; };

    // Create the playback device text
    sf::Text playbackDeviceText(font,
                                {.position      = {10.f, 566.f},
                                 .string        = "Current playback device: " + getCurrentDeviceName(),
                                 .characterSize = 20u,
                                 .fillColor     = {80, 80, 80}});

    // Create the playback device instructions text
    const sf::Text playbackDeviceInstructions(font,
                                              {.position      = {565.f, 566.f},
                                               .string        = "Press F1 to change device",
                                               .characterSize = 20u,
                                               .fillColor     = {80, 80, 80}});

    // Start the game loop
    const sf::Clock clock;
    while (true)
    {
        // Process events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                    // Left arrow key: previous effect
                    case sf::Keyboard::Key::Left:
                    {
                        effects[current]->stop();

                        if (current == 0)
                            current = effects.size() - 1;
                        else
                            --current;

                        effects[current]->start(getCurrentPlaybackDevice());

                        description.setString("Current effect: " + effects[current]->getName());
                        break;
                    }

                    // Right arrow key: next effect
                    case sf::Keyboard::Key::Right:
                    {
                        effects[current]->stop();

                        if (current == effects.size() - 1)
                            current = 0;
                        else
                            ++current;

                        effects[current]->start(getCurrentPlaybackDevice());

                        description.setString("Current effect: " + effects[current]->getName());
                        break;
                    }

                    // F1 key: change playback device
                    case sf::Keyboard::Key::F1:
                    {
                        std::size_t newPlaybackDeviceIndex{};

                        // We need to query the list every time we want to change
                        // since new devices could have been added in the mean time
                        auto newPlaybackDeviceHandles = sf::AudioContextUtils::getAvailablePlaybackDeviceHandles(
                            audioContext);

                        // TODO P1: cleanup
                        if (playbackDeviceHandles != newPlaybackDeviceHandles)
                        {
                            std::vector<sf::PlaybackDevice> newPlaybackDevices;
                            newPlaybackDevices.reserve(newPlaybackDeviceHandles.size());

                            for (const sf::PlaybackDeviceHandle& deviceHandle : newPlaybackDeviceHandles)
                            {
                                newPlaybackDevices.emplace_back(audioContext, deviceHandle);

                                if (deviceHandle.isDefault())
                                    newPlaybackDeviceIndex = newPlaybackDevices.size() - 1;
                            }

                            sf::PlaybackDevice& newPlaybackDevice = newPlaybackDevices[newPlaybackDeviceIndex];

                            getCurrentPlaybackDevice().transferResourcesTo(newPlaybackDevice);
                            effects[current]->start(newPlaybackDevice);

                            playbackDeviceHandles = std::move(newPlaybackDeviceHandles);
                            playbackDevices       = std::move(newPlaybackDevices);
                        }
                        else
                        {
                            newPlaybackDeviceIndex = (currentPlaybackDeviceIndex + 1) % playbackDevices.size();

                            sf::PlaybackDevice& newPlaybackDevice = playbackDevices.at(newPlaybackDeviceIndex);
                            getCurrentPlaybackDevice().transferResourcesTo(newPlaybackDevice);
                        }


                        currentPlaybackDeviceIndex = newPlaybackDeviceIndex;
                        playbackDeviceText.setString("Current playback device: " + getCurrentDeviceName());
                        break;
                    }

                    default:
                    {
                        effects[current]->handleKey(keyPressed->code);
                        break;
                    }
                }
            }
        }

        // Update the current example
        const auto [x, y] = sf::Mouse::getPosition(window).toVector2f().componentWiseDiv(window.getSize().toVector2f());
        effects[current]->update(clock.getElapsedTime().asSeconds(), x, y);

        // Clear the window
        window.clear({50, 50, 50});

        // Draw the current example
        window.draw(*effects[current]);

        // Draw the text
        window.draw(textBackgroundTexture, {.position = {0.f, 520.f}, .color = {255, 255, 255, 200}});
        window.draw(instructions);
        window.draw(description);
        window.draw(playbackDeviceText);
        window.draw(playbackDeviceInstructions);

        // Finally, display the rendered frame on screen
        window.display();
    }

    // Stop effect so that tone generators don't have to keep generating data while being destroyed
    effects[current]->stop();
}
