// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"


namespace hg::Steam
{

class SteamManager
{
private:
    class SteamManagerImpl;

    sf::base::UniquePtr<SteamManagerImpl> m_impl;

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
    bool unlockAchievement(sf::base::SizeT idx);
    bool isAchievementUnlocked(sf::base::SizeT idx);

    bool setRichPresenceInGame(sf::base::StringView levelNameFormat);

    bool               setAndStoreStat(sf::base::StringView name, int data);
    [[nodiscard]] bool getAchievement(bool* out, sf::base::StringView name);
    [[nodiscard]] bool getStat(int* out, sf::base::StringView name);

    bool updateHardcodedAchievements();

    bool isOnSteamDeck();
};

} // namespace hg::Steam
