// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/UniquePtr.hpp"


namespace hg::Steam
{

class SteamManager
{
private:
    class SteamManagerImpl;

    zb::UniquePtr<SteamManagerImpl> m_impl;

    [[nodiscard]] const SteamManagerImpl& impl() const noexcept;
    [[nodiscard]] SteamManagerImpl&       impl() noexcept;

public:
    explicit SteamManager();
    ~SteamManager();

    SteamManager(const SteamManager&)            = delete;
    SteamManager& operator=(const SteamManager&) = delete;

    SteamManager(SteamManager&&)            = delete;
    SteamManager& operator=(SteamManager&&) = delete;

    [[nodiscard]] bool isInitialized() const noexcept;

    bool requestStatsAndAchievements();

    bool runCallbacks();

    bool storeStats();
    bool unlockAchievement(zb::SizeT idx);
    bool isAchievementUnlocked(zb::SizeT idx);

    bool setRichPresenceInGame(zb::StringView levelNameFormat);

    bool               setAndStoreStat(zb::StringView name, int data);
    [[nodiscard]] bool getAchievement(bool* out, zb::StringView name);
    [[nodiscard]] bool getStat(int* out, zb::StringView name);

    bool updateHardcodedAchievements();

    bool isOnSteamDeck();
};

} // namespace hg::Steam
