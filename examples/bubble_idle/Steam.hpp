// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/StringView.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"


namespace hg::Steam
{

class SteamManager
{
private:
    class SteamManagerImpl;

    za::UniquePtr<SteamManagerImpl> m_impl;

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
    bool unlockAchievement(za::SizeT idx);
    bool isAchievementUnlocked(za::SizeT idx);

    bool setRichPresenceInGame(za::StringView levelNameFormat);

    bool               setAndStoreStat(za::StringView name, int data);
    [[nodiscard]] bool getAchievement(bool* out, za::StringView name);
    [[nodiscard]] bool getStat(int* out, za::StringView name);

    bool updateHardcodedAchievements();

    bool isOnSteamDeck();
};

} // namespace hg::Steam
