////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/AudioSettings.hpp"
#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/EffectProcessor.hpp"
#include "Zancle/Audio/Listener.hpp"
#include "Zancle/Audio/Music.hpp"
#include "Zancle/Audio/MusicReader.hpp"
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/PlaybackDeviceHandle.hpp"
#include "Zancle/Audio/SoundChannel.hpp"
#include "Zancle/Audio/SoundStream.hpp"
#include "Zancle/Graphics/CircleShape.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/ConvexShape.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/Transform.hpp"
#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Fmt/FmtPath.hpp"
#include "Zancle/System/IO.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Utf8String.hpp"
#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"
#include "ZancleBase/Abort.hpp"
#include "ZancleBase/Array.hpp"
#include "ZancleBase/Clamp.hpp"
#include "ZancleBase/Constants.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/Math/Floor.hpp"
#include "ZancleBase/Math/Fmod.hpp"
#include "ZancleBase/Math/Lround.hpp"
#include "ZancleBase/Math/Pow.hpp"
#include "ZancleBase/Math/Sin.hpp"
#include "ZancleBase/Math/Tan.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToString.hpp"
#include "ZancleBase/Vector.hpp"

#include "ExampleUtils/Scaling.hpp"

#include <limits>


namespace
{
constexpr auto windowWidth  = 800u;
constexpr auto windowHeight = 600u;

za::Path resourcesDir()
{
#ifdef ZA_SYSTEM_IOS
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

    [[nodiscard]] const zb::String& getName() const
    {
        return m_name;
    }

    virtual void update(float time, float x, float y)                          = 0;
    virtual void draw(za::RenderTarget& target, za::RenderStates states) const = 0;

    virtual void start(za::PlaybackDevice&, za::MusicReader&) = 0;
    virtual void stop()                                       = 0;

    virtual void handleKey([[maybe_unused]] za::Keyboard::Key key)
    {
    }

protected:
    explicit Effect(zb::String name) : m_name(ZB_MOVE(name))
    {
    }

private:
    zb::String m_name;
};


////////////////////////////////////////////////////////////
// Surround Sound / Positional Audio Effect / Attenuation
////////////////////////////////////////////////////////////
class Surround : public Effect
{
public:
    explicit Surround(za::Listener& listener) : Effect("Surround / Attenuation"), m_listener(listener)
    {
        m_listenerShape.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f};
    }

    void update(float /*time*/, float x, float y) override
    {
        m_position = {windowWidth * x - 10.f, windowHeight * y - 10.f};
        m_music->setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        target.draw(m_listenerShape, states);

        states.transform = za::Transform::Identity;
        states.transform.translate(m_position);

        target.draw(m_soundShape, states);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        m_music
            .emplace(playbackDevice,
                     musicReader,
                     za::AudioSettings{
                         .attenuation = 0.04f,
                         .looping     = true,
                     })
            .play();
    }

    void stop() override
    {
        m_music.reset();
    }

private:
    za::Listener&           m_listener;
    za::CircleShape         m_listenerShape{{.origin = {10.f, 10.f}, .fillColor = za::Color::Red, .radius = 20.f}};
    za::CircleShape         m_soundShape{{.origin = {10.f, 10.f}, .radius = 20.f}};
    za::Vec2f               m_position;
    zb::Optional<za::Music> m_music;
};


////////////////////////////////////////////////////////////
// Pitch / Volume Effect
////////////////////////////////////////////////////////////
class PitchVolume : public Effect
{
public:
    explicit PitchVolume(za::Listener& listener, const za::Font& font) :
        Effect("Pitch / Volume"),
        m_listener(listener),
        m_pitchText(font,
                    {
                        .position      = {windowWidth / 2.f - 120.f, windowHeight / 2.f - 80.f},
                        .string        = "",
                        .characterSize = 30u,
                    }),
        m_volumeText(font,
                     {
                         .position      = {windowWidth / 2.f - 120.f, windowHeight / 2.f - 30.f},
                         .string        = "",
                         .characterSize = 30u,
                     })
    {
    }

    void update(float /*time*/, float x, float y) override
    {
        m_pitch  = zb::clamp(2.f * x, 0.f, 2.f);
        m_volume = zb::clamp(100.f * (1.f - y), 0.f, 100.f);

        m_music->setPitch(m_pitch);
        m_music->setVolume(m_volume / 100.f);

        m_pitchText.setString("Pitch: " + zb::toString(m_pitch));
        m_volumeText.setString("Volume: " + zb::toString(m_volume));
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        target.withRenderStates(states).drawAll(m_pitchText, m_volumeText);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        // We set the listener position back to the default
        // so that the music is right on top of the listener
        m_listener.position = {0.f, 0.f, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        m_music
            .emplace(playbackDevice,
                     musicReader,
                     za::AudioSettings{
                         .volume      = m_volume / 100.f,
                         .pitch       = m_pitch,
                         .attenuation = 0.f,
                         .looping     = true,
                     })
            .play();
    }

    void stop() override
    {
        m_music.reset();
    }

private:
    za::Listener&           m_listener;
    float                   m_pitch{1.f};
    float                   m_volume{100.f};
    za::Text                m_pitchText;
    za::Text                m_volumeText;
    zb::Optional<za::Music> m_music;
};


////////////////////////////////////////////////////////////
// Attenuation Effect
////////////////////////////////////////////////////////////
class Attenuation : public Effect
{
public:
    explicit Attenuation(za::Listener& listener, const za::Font& font) :
        Effect("Attenuation"),
        m_listener(listener),
        m_text(font,
               {.position = {20.f, 20.f},
                .string   = "Attenuation factor dampens full volume of sound while within inner cone based on distance "
                            "to "
                            "listener.\nCone outer gain determines volume of sound while outside outer cone.\nWhen "
                            "within "
                            "outer cone, volume is linearly interpolated between inner and outer volumes.",
                .characterSize = 18u})
    {
    }

    void update(float /*time*/, float x, float y) override
    {
        m_position = {windowWidth * x - 10.f, windowHeight * y - 10.f};
        m_music->setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        auto statesCopy(states);

        statesCopy.transform = za::Transform::Identity;
        statesCopy.transform.translate(m_position);

        target.withRenderStates(statesCopy).drawAll(m_soundConeOuter, m_soundConeInner, m_soundShape);
        target.withRenderStates(states).drawAll(m_listenerShape, m_text);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        // Sound cone parameters
        static constexpr auto coneHeight     = windowHeight * 2.f;
        static constexpr auto outerConeAngle = za::degrees(120.f);
        static constexpr auto innerConeAngle = za::degrees(30.f);

        // Set common properties of both cones
        for (za::ConvexShape* cone : {&m_soundConeOuter, &m_soundConeInner})
            cone->setPoint(0, {0.f, 0.f});

        // Make each cone based on their angle and height
        static constexpr auto makeCone = [](auto& shape, const auto& angle)
        {
            const auto theta = za::degrees(90.f) - (angle / 2);
            const auto x     = coneHeight / zb::tan(theta.asRadians());

            shape.setPoint(1, {-x, coneHeight});
            shape.setPoint(2, {x, coneHeight});
        };

        makeCone(m_soundConeOuter, outerConeAngle);
        makeCone(m_soundConeInner, innerConeAngle);

        m_music
            .emplace(playbackDevice,
                     musicReader,
                     za::AudioSettings{
                         .cone        = {innerConeAngle, outerConeAngle, 0.f},
                         .direction   = {0.f, 1.f, 0.f},
                         .attenuation = m_attenuation,
                         .looping     = true,
                     })
            .play();
    }

    void stop() override
    {
        m_music.reset();
    }

private:
    za::Listener&   m_listener;
    za::CircleShape m_listenerShape{
        {.position  = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f + 100.f},
         .origin    = {10.f, 10.f},
         .fillColor = za::Color::Red,
         .radius    = 20.f}};
    za::CircleShape m_soundShape{{.origin = {10.f, 10.f}, .fillColor = za::Color::Magenta, .radius = 20.f}};
    za::ConvexShape m_soundConeOuter{{.position = {10.f, 10.f}, .fillColor = za::Color::Black, .pointCount = 3u}};
    za::ConvexShape m_soundConeInner{{.position = {10.f, 10.f}, .fillColor = za::Color::Cyan, .pointCount = 3u}};
    za::Text        m_text;
    za::Vec2f       m_position;
    zb::Optional<za::Music> m_music;

    float m_attenuation{0.01f};
};


////////////////////////////////////////////////////////////
// Tone Generator
////////////////////////////////////////////////////////////
class Tone : public Effect
{
private:
    enum class Type : zb::SizeT
    {
        Sine,
        Square,
        Triangle,
        Sawtooth
    };

    struct ToneState
    {
        Tone& tone;

        bool onGetData(zb::Vector<zb::I16>& outBuffer)
        {
            const auto period = 1.f / tone.m_frequency;

            outBuffer.resize(chunkSize);

            for (auto i = 0u; i < chunkSize; ++i)
            {
                auto value = 0.f;

                switch (tone.m_type)
                {
                    case Type::Sine:
                    {
                        value = tone.m_amplitude * zb::sin(2 * zb::pi * tone.m_frequency * tone.m_time);
                        break;
                    }
                    case Type::Square:
                    {
                        value = tone.m_amplitude * (2 * (2 * zb::floor(tone.m_frequency * tone.m_time) -
                                                         zb::floor(2 * tone.m_frequency * tone.m_time)) +
                                                    1);
                        break;
                    }
                    case Type::Triangle:
                    {
                        value = 4 * tone.m_amplitude / period *
                                    zb::fabs(zb::fmod(((zb::fmod((tone.m_time - period / 4), period)) + period), period) -
                                             period / 2) -
                                tone.m_amplitude;
                        break;
                    }
                    case Type::Sawtooth:
                    {
                        value = tone.m_amplitude * 2 * (tone.m_time / period - zb::floor(0.5f + tone.m_time / period));
                        break;
                    }
                }

                outBuffer[i] = static_cast<zb::I16>(zb::lround(value * std::numeric_limits<zb::I16>::max()));
                tone.m_time += timePerSample;
            }

            return true;
        }
    };

public:
    explicit Tone(za::Listener& listener, const za::Font& font) :
        Effect("Tone Generator"),
        m_listener(listener),
        m_instruction(font,
                      {
                          .position      = {windowWidth / 2.f - 370.f, windowHeight / 2.f - 200.f},
                          .string        = "Press up and down arrows to change the current wave type",
                          .characterSize = 30,
                      }),
        m_currentType(font,
                      {
                          .position      = {windowWidth / 2.f - 150.f, windowHeight / 2.f - 100.f},
                          .string        = "",
                          .characterSize = 30u,
                      }),
        m_currentAmplitude(font,
                           {
                               .position      = {windowWidth / 2.f - 150.f, windowHeight / 2.f - 50.f},
                               .string        = "",
                               .characterSize = 30u,
                           }),
        m_currentFrequency(font,
                           {
                               .position      = {windowWidth / 2.f - 150.f, windowHeight / 2.f},
                               .string        = "",
                               .characterSize = 30u,
                           })
    {
    }

    void update(float /*time*/, float x, float y) override
    {
        m_amplitude = zb::clamp(0.2f * (1.f - y), 0.f, 0.2f);
        m_frequency = zb::clamp(500.f * x, 0.f, 500.f);

        m_currentAmplitude.setString("Amplitude: " + zb::toString(m_amplitude));
        m_currentFrequency.setString("Frequency: " + zb::toString(m_frequency) + " Hz");

        m_currentType.setString(zb::String{"Wave Type: "} +
                                zb::Array{"Sine", "Square", "Triangle", "Sawtooth"}[static_cast<zb::SizeT>(m_type)]);
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        target.withRenderStates(states).drawAll(m_instruction, m_currentType, m_currentAmplitude, m_currentFrequency);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader&) override
    {
        // We set the listener position back to the default
        // so that the tone is right on top of the listener
        m_listener.position = {0.f, 0.f, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        m_toneSoundStream.emplace(playbackDevice, za::ChannelMap{za::SoundChannel::Mono}, sampleRate, *this).play();
    }

    void stop() override
    {
        m_toneSoundStream.reset();
    }

    void handleKey(za::Keyboard::Key key) override
    {
        if (key == za::Keyboard::Key::Down)
            m_type = static_cast<Type>((static_cast<zb::SizeT>(m_type) + 1) % 4u); // Forward
        else if (key == za::Keyboard::Key::Up)
            m_type = static_cast<Type>((static_cast<zb::SizeT>(m_type) - 1) % 4u); // Reverse
    }

private:
    static constexpr unsigned int sampleRate{44'100};
    static constexpr zb::SizeT    chunkSize{sampleRate / 100};
    static constexpr float        timePerSample{1.f / float{sampleRate}};

    za::Listener& m_listener;

    Type  m_type{Type::Triangle};
    float m_amplitude{0.05f};
    float m_frequency{220};
    float m_time{};

    za::Text m_instruction;
    za::Text m_currentType;
    za::Text m_currentAmplitude;
    za::Text m_currentFrequency;

    zb::Optional<za::SoundStream<ToneState>> m_toneSoundStream;
};


////////////////////////////////////////////////////////////
// Dopper Shift Effect
////////////////////////////////////////////////////////////
class Doppler : public Effect
{
private:
    struct DopplerState
    {
        Doppler& doppler;

        bool onGetData(zb::Vector<zb::I16>& outBuffer)
        {
            const auto period = 1.f / doppler.m_frequency;

            outBuffer.resize(chunkSize);

            for (auto i = 0u; i < chunkSize; ++i)
            {
                const auto value = doppler.m_amplitude * 2 *
                                   (doppler.m_time / period - zb::floor(0.5f + doppler.m_time / period));

                outBuffer[i] = static_cast<zb::I16>(zb::lround(value * std::numeric_limits<zb::I16>::max()));
                doppler.m_time += timePerSample;
            }

            return true;
        }
    };

public:
    explicit Doppler(za::Listener& listener, const za::Font& font) :
        Effect("Doppler Shift"),
        m_listener(listener),
        m_currentVelocity(font,
                          {
                              .position      = {windowWidth / 2.f - 150.f, windowHeight * 3.f / 4.f - 50.f},
                              .string        = "",
                              .characterSize = 30u,
                          }),
        m_currentFactor(font,
                        {
                            .position      = {windowWidth / 2.f - 150.f, windowHeight * 3.f / 4.f},
                            .string        = "",
                            .characterSize = 30u,
                        })
    {
        m_position.y = (windowHeight - 20.f) / 2.f - 40.f;
    }

    void update(float time, float x, float y) override
    {
        m_velocity = zb::clamp(150.f * (1.f - y), 0.f, 150.f);
        m_factor   = zb::clamp(x, 0.f, 1.f);

        m_currentVelocity.setString("Velocity: " + zb::toString(m_velocity));
        m_currentFactor.setString("Doppler Factor: " + zb::toString(m_factor));

        m_position.x = zb::fmod(time, 8.f) * windowWidth / 8.f;

        m_dopplerSoundStream->setPosition({m_position.x, m_position.y, 0.f});
        m_dopplerSoundStream->setVelocity({m_velocity, 0.f, 0.f});
        m_dopplerSoundStream->setDopplerFactor(m_factor);
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        auto statesCopy(states);

        statesCopy.transform = za::Transform::Identity;
        statesCopy.transform.translate(m_position - za::Vec2f({20.f, 0.f}));

        target.draw(m_listenerShape, states);
        target.draw(m_soundShape, statesCopy);
        target.draw(m_currentVelocity, states);
        target.draw(m_currentFactor, states);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader&) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        auto& stream = m_dopplerSoundStream.emplace(playbackDevice, za::ChannelMap{za::SoundChannel::Mono}, sampleRate, *this);
        stream.setAttenuation(0.05f);
        stream.play();
    }

    void stop() override
    {
        m_dopplerSoundStream.reset();
    }

private:
    static constexpr unsigned int sampleRate{44'100};
    static constexpr zb::SizeT    chunkSize{sampleRate / 100};
    static constexpr float        timePerSample{1.f / float{sampleRate}};

    za::Listener& m_listener;

    float m_amplitude{0.05f};
    float m_frequency{220};
    float m_time{};

    float           m_velocity{0.f};
    float           m_factor{1.f};
    za::CircleShape m_listenerShape{
        {.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f}, .fillColor = za::Color::Red, .radius = 20.f}};
    za::CircleShape m_soundShape{{.origin = {10.f, 10.f}, .radius = 20.f}};
    za::Vec2f       m_position;
    za::Text        m_currentVelocity;
    za::Text        m_currentFactor;

    zb::Optional<za::SoundStream<DopplerState>> m_dopplerSoundStream;
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
        m_music->setPosition({m_position.x, m_position.y, 0.f});
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const override
    {
        target.draw(m_listenerShape, states);
        target.draw(m_enabledText, states);
        target.draw(m_instructions, states);

        states.transform = za::Transform::Identity;
        states.transform.translate(m_position);

        target.draw(m_soundShape, states);
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        // Synchronize listener audio position with graphical position
        m_listener.position = {m_listenerShape.position.x, m_listenerShape.position.y, 0.f};

        if (!playbackDevice.applyListener(m_listener))
            zb::printErrLn("Failed to update listener");

        m_music
            .emplace(playbackDevice,
                     musicReader,
                     za::AudioSettings{
                         .attenuation = 0.f,
                         .looping     = true,
                     })
            .play();
    }

    void stop() override
    {
        m_music.reset();
    }

protected:
    explicit Processing(za::Listener& listener, const za::Font& font, zb::String name) :
        Effect(ZB_MOVE(name)),
        m_listener(listener),
        m_enabledText(font, {.string = "Processing: Enabled", .characterSize = 30u}),
        m_instructions(font, {.string = "Press Space to enable/disable processing", .characterSize = 30u})
    {
        m_listenerShape.position = {(windowWidth - 20.f) / 2.f, (windowHeight - 20.f) / 2.f};

        m_enabledText.position  = {windowWidth / 2.f - 120.f, windowHeight * 3.f / 4.f - 50.f};
        m_instructions.position = {windowWidth / 2.f - 250.f, windowHeight * 3.f / 4.f};
    }


    za::Listener& m_listener;

    // Needs to be defined above `m_music` because it's used in the music's effect processor
    bool m_enabled{true};

    zb::Optional<za::Music> m_music;

private:
    void handleKey(za::Keyboard::Key key) override
    {
        if (key == za::Keyboard::Key::Space)
            m_enabled = !m_enabled;

        m_enabledText.setString(m_enabled ? "Processing: Enabled" : "Processing: Disabled");
    }

    za::CircleShape m_listenerShape{{.origin = {10.f, 10.f}, .fillColor = za::Color::Red, .radius = 20.f}};
    za::CircleShape m_soundShape{{.origin = {10.f, 10.f}, .radius = 20.f}};
    za::Vec2f       m_position;
    za::Text        m_enabledText;
    za::Text        m_instructions;
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
        const bool success = m_music->setEffectProcessor(
            [coefficients, &enabled = m_enabled, state = zb::Vector<State>()](const float*  inputFrames,
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

        if (!success)
        {
            zb::printErrLn("Failed to set effect processor");
            zb::abort();
        }
    }
};


////////////////////////////////////////////////////////////
// High-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct HighPassFilter : BiquadFilter
{
    explicit HighPassFilter(za::Listener& listener, const za::Font& font) :
        BiquadFilter(listener, font, "High-pass Filter")
    {
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        BiquadFilter::start(playbackDevice, musicReader);

        static constexpr auto cutoffFrequency = 2000.f;

        const auto c = zb::tan(zb::pi * cutoffFrequency / static_cast<float>(playbackDevice.getSampleRate()));

        Coefficients coefficients{.a0 = 1.f / (1.f + zb::sqrt2 * c + zb::pow(c, 2.f)),
                                  .a1 = -2.f * coefficients.a0,
                                  .a2 = coefficients.a0,
                                  .b1 = 2.f * coefficients.a0 * (zb::pow(c, 2.f) - 1.f),
                                  .b2 = coefficients.a0 * (1.f - zb::sqrt2 * c + zb::pow(c, 2.f))};

        setCoefficients(coefficients);
    }
};


////////////////////////////////////////////////////////////
// Low-pass Filter (https://github.com/dimtass/DSP-Cpp-filters)
////////////////////////////////////////////////////////////
struct LowPassFilter : BiquadFilter
{
    explicit LowPassFilter(za::Listener& listener, const za::Font& font) :
        BiquadFilter(listener, font, "Low-pass Filter")
    {
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        BiquadFilter::start(playbackDevice, musicReader);

        static constexpr auto cutoffFrequency = 500.f;

        const auto c = 1.f / zb::tan(zb::pi * cutoffFrequency / static_cast<float>(playbackDevice.getSampleRate()));

        Coefficients coefficients{.a0 = 1.f / (1.f + zb::sqrt2 * c + zb::pow(c, 2.f)),
                                  .a1 = 2.f * coefficients.a0,
                                  .a2 = coefficients.a0,
                                  .b1 = 2.f * coefficients.a0 * (1.f - zb::pow(c, 2.f)),
                                  .b2 = coefficients.a0 * (1.f - zb::sqrt2 * c + zb::pow(c, 2.f))};

        setCoefficients(coefficients);
    }
};


////////////////////////////////////////////////////////////
// Echo (miniaudio implementation)
////////////////////////////////////////////////////////////
struct Echo : Processing
{
    explicit Echo(za::Listener& listener, const za::Font& font) : Processing(listener, font, "Echo")
    {
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        Processing::start(playbackDevice, musicReader);

        static constexpr auto delay = 0.2f;
        static constexpr auto decay = 0.75f;
        static constexpr auto wet   = 0.8f;
        static constexpr auto dry   = 1.f;

        const auto sampleRate    = musicReader.getSampleRate();
        const auto delayInFrames = static_cast<unsigned int>(static_cast<float>(sampleRate) * delay);

        // We use a mutable lambda to tie the lifetime of the state to the lambda itself
        // This is necessary since the Echo object will be destroyed before the music object
        // While the music object exists, it is possible that the audio engine will try to call
        // this lambda hence we need to always have a usable state until the music and the
        // associated lambda are destroyed
        const bool success = m_music->setEffectProcessor(
            [delayInFrames,
             &enabled = m_enabled,
             buffer   = zb::Vector<float>(),
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

        if (!success)
        {
            zb::printErrLn("Failed to set effect processor");
            zb::abort();
        }
    }
};


////////////////////////////////////////////////////////////
// Reverb (https://github.com/sellicott/DSP-FFMpeg-Reverb)
////////////////////////////////////////////////////////////
class Reverb : public Processing
{
public:
    explicit Reverb(za::Listener& listener, const za::Font& font) : Processing(listener, font, "Reverb")
    {
    }

    void start(za::PlaybackDevice& playbackDevice, za::MusicReader& musicReader) override
    {
        Processing::start(playbackDevice, musicReader);

        static constexpr auto sustain = 0.7f; // [0.f; 1.f]

        // We use a mutable lambda to tie the lifetime of the state to the lambda itself
        // This is necessary since the Echo object will be destroyed before the music object
        // While the music object exists, it is possible that the audio engine will try to call
        // this lambda hence we need to always have a usable state until the music and the
        // associated lambda are destroyed
        const bool success = m_music->setEffectProcessor(
            [sampleRate = playbackDevice.getSampleRate(),
             filters    = zb::Vector<ReverbFilter<float>>(),
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
                filters.emplaceBack(sampleRate, sustain);

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

        if (!success)
        {
            zb::printErrLn("Failed to set effect processor");
            zb::abort();
        }
    }

private:
    template <typename T>
    class AllPassFilter
    {
    public:
        AllPassFilter(zb::SizeT delay, float theGain) : m_buffer(delay, {}), m_gain(theGain)
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
        zb::Vector<T> m_buffer;
        zb::SizeT     m_cursor{};
        const float   m_gain{};
    };

    template <typename T>
    class FIRFilter
    {
    public:
        explicit FIRFilter(zb::Vector<float> taps) : m_taps(ZB_MOVE(taps))
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
        const zb::Vector<float> m_taps;
        zb::Vector<T>           m_buffer = zb::Vector<T>(m_taps.size(), {});
        zb::SizeT               m_cursor{};
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
        AllPassFilter<T> m_allPass[4];
        FIRFilter<T>     m_fir;
        zb::Vector<T>    m_buffer;
        zb::SizeT        m_cursor{};
        const zb::SizeT  m_interval{m_buffer.size() / 3};
        const float      m_feedbackGain{};
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
    auto graphicsContext = za::GraphicsContext::create().value();

    // Create the main window
    constexpr za::Vec2f windowSize{windowWidth, windowHeight};

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size      = windowSize.toVec2u(),
                          .title     = "Zancle Sound Effects",
                          .resizable = true,
                          .vsync     = true,
                      })
                      .value();

    auto windowView = computeAspectRatioAwareView(window.getSize().toVec2f(), windowSize);

    // Load the application font and pass it to the Effect class
    const auto font = za::Font::openFromFile(resourcesDir() / "tuffy.ttf").value();

    // Exit early if music file not found
    const auto musicPath = resourcesDir() / "doodle_pop.ogg";
    if (!musicPath.exists())
    {
        zb::printErrLn("Music file '{}' not found, aborting", musicPath);
        return 1;
    }

    // Create the playback device and listener
    // TODO P1: docs
    auto audioContext          = za::AudioContext::create().value();
    auto playbackDeviceHandles = za::AudioContext::getAvailablePlaybackDeviceHandles();

    zb::SizeT currentPlaybackDeviceIndex = 0;

    zb::InPlaceVector<za::PlaybackDevice, 8> playbackDevices;
    playbackDevices.reserve(playbackDeviceHandles.size());

    for (const za::PlaybackDeviceHandle& deviceHandle : playbackDeviceHandles)
    {
        playbackDevices.emplaceBack(deviceHandle);

        if (deviceHandle.isDefault())
            currentPlaybackDeviceIndex = playbackDevices.size() - 1;
    }

    const auto getCurrentPlaybackDevice = [&] -> za::PlaybackDevice&
    { return playbackDevices[currentPlaybackDeviceIndex]; };

    // TODO P1: docs
    za::Listener listener;

    // Helper function to open a new instance of the music file
    auto musicReader = za::MusicReader::openFromFile(musicPath).value();

    // Create the effects
    Surround       surroundEffect(listener);
    PitchVolume    pitchVolumeEffect(listener, font);
    Attenuation    attenuationEffect(listener, font);
    Tone           toneEffect(listener, font);
    Doppler        dopplerEffect(listener, font);
    HighPassFilter highPassFilterEffect(listener, font);
    LowPassFilter  lowPassFilterEffect(listener, font);
    Echo           echoEffect(listener, font);
    Reverb         reverbEffect(listener, font);

    const zb::Array<Effect*, 9> effects{&surroundEffect,
                                        &pitchVolumeEffect,
                                        &attenuationEffect,
                                        &toneEffect,
                                        &dopplerEffect,
                                        &highPassFilterEffect,
                                        &lowPassFilterEffect,
                                        &echoEffect,
                                        &reverbEffect};

    zb::SizeT current = 0;

    effects[current]->start(getCurrentPlaybackDevice(), musicReader);

    // Create the messages background
    const auto textBackgroundTexture = za::Texture::loadFromFile(resourcesDir() / "text-background.png").value();

    // Create the description text
    za::Text description(font,
                         {.position      = {10.f, 522.f},
                          .string        = "Current effect: " + effects[current]->getName(),
                          .characterSize = 20u,
                          .fillColor     = {80, 80, 80}});

    // Create the instructions text
    const za::Text instructions(font,
                                {.position      = {280.f, 544.f},
                                 .string        = "Press left and right arrows to change the current effect",
                                 .characterSize = 20u,
                                 .fillColor     = {80, 80, 80}});

    // Utility functions
    const auto getCurrentDeviceName = [&] { return zb::String{getCurrentPlaybackDevice().getDeviceHandle().getName()}; };

    // Create the playback device text
    za::Text playbackDeviceText(font,
                                {.position      = {10.f, 566.f},
                                 .string        = "Current playback device: " + getCurrentDeviceName(),
                                 .characterSize = 20u,
                                 .fillColor     = {80, 80, 80}});

    // Create the playback device instructions text
    const za::Text playbackDeviceInstructions(font,
                                              {.position      = {565.f, 566.f},
                                               .string        = "Press F1 to change device",
                                               .characterSize = 20u,
                                               .fillColor     = {80, 80, 80}});

    // Start the game loop
    const za::Clock clock;
    while (true)
    {
        // Process events
        while (const zb::Optional event = window.pollEvent())
        {
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, windowView))
                continue;

            if (const auto* keyPressed = event->getIf<za::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                    // Left arrow key: previous effect
                    case za::Keyboard::Key::Left:
                    {
                        effects[current]->stop();

                        if (current == 0)
                            current = effects.size() - 1;
                        else
                            --current;

                        effects[current]->start(getCurrentPlaybackDevice(), musicReader);

                        description.setString("Current effect: " + effects[current]->getName());
                        break;
                    }

                    // Right arrow key: next effect
                    case za::Keyboard::Key::Right:
                    {
                        effects[current]->stop();

                        if (current == effects.size() - 1)
                            current = 0;
                        else
                            ++current;

                        effects[current]->start(getCurrentPlaybackDevice(), musicReader);

                        description.setString("Current effect: " + effects[current]->getName());
                        break;
                    }

                    // F1 key: change playback device
                    case za::Keyboard::Key::F1:
                    {
                        zb::SizeT newPlaybackDeviceIndex{};

                        // We need to query the list every time we want to change
                        // since new devices could have been added in the mean time
                        auto newPlaybackDeviceHandles = za::AudioContext::getAvailablePlaybackDeviceHandles();

                        // TODO P1: cleanup
                        if (playbackDeviceHandles != newPlaybackDeviceHandles)
                        {
                            effects[current]->stop();
                            playbackDevices.clear();

                            for (const za::PlaybackDeviceHandle& deviceHandle : newPlaybackDeviceHandles)
                            {
                                playbackDevices.emplaceBack(deviceHandle);

                                if (deviceHandle.isDefault())
                                    newPlaybackDeviceIndex = playbackDevices.size() - 1;
                            }

                            za::PlaybackDevice& newPlaybackDevice = playbackDevices[newPlaybackDeviceIndex];

                            effects[current]->stop();
                            effects[current]->start(newPlaybackDevice, musicReader);

                            playbackDeviceHandles = ZB_MOVE(newPlaybackDeviceHandles);
                        }
                        else
                        {
                            newPlaybackDeviceIndex = (currentPlaybackDeviceIndex + 1) % playbackDevices.size();

                            za::PlaybackDevice& newPlaybackDevice = playbackDevices[newPlaybackDeviceIndex];

                            effects[current]->stop();
                            effects[current]->start(newPlaybackDevice, musicReader);
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
        const auto [x,
                    y] = windowView.screenToWorld(za::Mouse::getPosition(window).toVec2f(), window.getSize().toVec2f())
                             .componentWiseDiv(windowSize.toVec2f());

        effects[current]->update(clock.getElapsedTime().asSeconds(), x, y);

        // Clear the window
        window.clear({50, 50, 50});

        window.withRenderStates({.view = windowView})
            .draw(*effects[current])
            .draw(textBackgroundTexture, {.position = {0.f, 520.f}, .color = {255, 255, 255, 200}})
            .drawAll(instructions, description, playbackDeviceText, playbackDeviceInstructions);

        // Finally, display the rendered frame on screen
        window.display();
    }

    // Stop effect so that tone generators don't have to keep generating data while being destroyed
    effects[current]->stop();
}
