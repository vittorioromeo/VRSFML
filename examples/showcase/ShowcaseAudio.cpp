#include "ShowcaseAudio.hpp"
#include "ShowcaseExample.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"

#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/SizeT.hpp"

#include <cstdio>


////////////////////////////////////////////////////////////
void ExampleAudio::refreshPlaybackDevices()
{
    // Sounds and musics must be destroyed *before* playback devices
    m_activeSounds.clear();
    m_activeMusic.reset();

    m_playbackDevices.clear();

    for (const auto& playbackDeviceHandle : sf::AudioContext::getAvailablePlaybackDeviceHandles())
        m_playbackDevices.emplaceBack(playbackDeviceHandle);
}


////////////////////////////////////////////////////////////
ExampleAudio::ExampleAudio() : ShowcaseExample{"Audio"}
{
    refreshPlaybackDevices();
}


////////////////////////////////////////////////////////////
void ExampleAudio::update([[maybe_unused]] const float deltaTimeMs)
{
}


////////////////////////////////////////////////////////////
void ExampleAudio::imgui()
{
    ImGui::Begin("Audio Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Playback devices:");

    sf::base::SizeT i = 0u;
    for (auto& playbackDevice : m_playbackDevices)
    {
        ImGui::Text("%s", playbackDevice.getName());
        ImGui::SameLine();
        ImGui::Text("%s", playbackDevice.isDefault() ? "(default)" : "");
        ImGui::SameLine();

        char buttonLabel[64];

        std::snprintf(buttonLabel, sizeof(buttonLabel), "Play Sound##%zu", i);
        if (ImGui::Button(buttonLabel))
        {
            auto* const it = sf::base::findIf( //
                m_activeSounds.begin(),
                m_activeSounds.end(),
                [](const sf::Sound& sound) { return !sound.isPlaying(); });

            if (it != m_activeSounds.end())
            {
                if (&it->getPlaybackDevice() == &playbackDevice)
                    it->play();
                else
                    m_activeSounds.reEmplaceByIterator(it, playbackDevice, m_sbByteMeow).play();
            }
            else if (m_activeSounds.size() < 32u)
                m_activeSounds.emplaceBack(playbackDevice, m_sbByteMeow).play();
        }

        ImGui::SameLine();

        std::snprintf(buttonLabel, sizeof(buttonLabel), "Play Music##%zu", i);
        if (ImGui::Button(buttonLabel))
        {
            if (!m_activeMusic.hasValue())
                m_activeMusic.emplace(playbackDevice, m_msBGMWizard).play();
            else if (&m_activeMusic->getPlaybackDevice() == &playbackDevice)
                m_activeMusic->resume();
            else
            {
                const auto playingOffset = m_activeMusic->getPlayingOffset();
                m_activeMusic.emplace(playbackDevice, m_msBGMWizard).play();
                m_activeMusic->setPlayingOffset(playingOffset);
            }
        }

        ++i;
    }

    if (ImGui::Button("Refresh playback devices"))
        refreshPlaybackDevices();

    ImGui::SameLine();

    if (ImGui::Button("Pause Music"))
        if (m_activeMusic.hasValue())
            m_activeMusic->pause();

    ImGui::SameLine();

    if (ImGui::Button("Stop Music"))
        m_activeMusic.reset();

    if (ImGui::Button("Switch Music Source"))
    {
        if (m_useAlternativeMusicSource)
            m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwizard.mp3").value();
        else
            m_msBGMWizard = sf::MusicReader::openFromFile("resources/bgmwitch.mp3").value();

        m_useAlternativeMusicSource = !m_useAlternativeMusicSource;
    }

    ImGui::End();
}


////////////////////////////////////////////////////////////
void ExampleAudio::draw()
{
}
