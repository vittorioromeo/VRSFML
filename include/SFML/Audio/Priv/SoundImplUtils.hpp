#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"

#include <miniaudio.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct SoundImplUtils
{
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    static void playImpl(TSelf& self, PlaybackDevice& playbackDevice)
    {
        if (self.m_impl->lastUsedPlaybackDevice != &playbackDevice)
            self.m_impl->soundBase.reset();

        self.m_impl->lastUsedPlaybackDevice = &playbackDevice;

        // TODO P0: should resume instead of restart on playback device transfer?

        if (!self.m_impl->soundBase.hasValue())
        {
            self.m_impl->soundBase.emplace(playbackDevice, &TSelf::Impl::vtable);
            self.m_impl->initialize();

            self.applySavedSettings(self.m_impl->soundBase->getSound());
            self.setEffectProcessor(self.getEffectProcessor());
            self.setPlayingOffset(self.getPlayingOffset());
        }

        if (self.m_impl->status == SoundSource::Status::Playing)
            self.setPlayingOffset(Time{});

        self.resumeOnLastPlaybackDevice();
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    static bool resumeOnLastPlaybackDeviceImpl(TSelf& self)
    {
        SFML_BASE_ASSERT(self.m_impl->soundBase.hasValue());

        if (const ma_result result = ma_sound_start(&self.m_impl->soundBase->getSound()); result != MA_SUCCESS)
            return priv::MiniaudioUtils::fail("start playing sound/soundstream", result);

        self.m_impl->status = SoundSource::Status::Playing;
        return true;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    static bool pauseImpl(TSelf& self)
    {
        if (!self.m_impl->soundBase.hasValue())
            return false;

        if (const ma_result result = ma_sound_stop(&self.m_impl->soundBase->getSound()); result != MA_SUCCESS)
            return priv::MiniaudioUtils::fail("stop playing sound/soundstream", result);

        if (self.m_impl->status == SoundSource::Status::Playing)
            self.m_impl->status = SoundSource::Status::Paused;

        return true;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    static bool stopImpl(TSelf& self)
    {
        if (!self.m_impl->soundBase.hasValue())
            return false;

        if (const ma_result result = ma_sound_stop(&self.m_impl->soundBase->getSound()); result != MA_SUCCESS)
            return priv::MiniaudioUtils::fail("stop playing sound/soundstream", result);

        self.setPlayingOffset(Time{});
        self.m_impl->status = SoundSource::Status::Stopped;

        return true;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    [[nodiscard]] static Time getPlayingOffsetImpl(const TSelf& self, const unsigned int channelCount, const unsigned int sampleRate)
    {
        if (channelCount == 0u || sampleRate == 0u || !self.m_impl->soundBase.hasValue())
            return Time{};

        // TODO P0: const bs
        return priv::MiniaudioUtils::getPlayingOffset(const_cast<TSelf&>(self).m_impl->soundBase->getSound());
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelf>
    [[nodiscard]] static base::Optional<base::U64> setPlayingOffsetImpl(TSelf& self, const Time playingOffset)
    {
        if (!self.m_impl->soundBase.hasValue())
            return base::nullOpt;

        if (self.m_impl->soundBase->getSound().pDataSource == nullptr ||
            self.m_impl->soundBase->getSound().engineNode.pEngine == nullptr)
            return base::nullOpt;

        return base::makeOptional<base::U64>(
            priv::MiniaudioUtils::getFrameIndex(self.m_impl->soundBase->getSound(), playingOffset));
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename TSelfImpl>
    static bool implInitializeImpl(TSelfImpl& selfImpl, const ChannelMap& channelMap)
    {
        SFML_BASE_ASSERT(selfImpl.soundBase.hasValue());

        if (!selfImpl.soundBase->initialize(&TSelfImpl::onEnd))
        {
            priv::err() << "Failed to initialize Sound/SoundStream::Impl";
            return false;
        }

        // Because we are providing a custom data source, we have to provide the channel map ourselves
        if (channelMap.isEmpty())
        {
            selfImpl.soundBase->getSound().engineNode.spatializer.pChannelMapIn = nullptr;
            return false;
        }

        selfImpl.soundBase->clearSoundChannelMap();

        for (const SoundChannel channel : channelMap)
            selfImpl.soundBase->addToSoundChannelMap(priv::MiniaudioUtils::soundChannelToMiniaudioChannel(channel));

        selfImpl.soundBase->refreshSoundChannelMap();

        return true;
    }
};

} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \class sf::SoundImplUtils
/// \ingroup audio
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
