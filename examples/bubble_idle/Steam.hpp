// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/UniquePtr.hpp"

#include <string_view>


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
    bool unlockAchievement(std::size_t idx);

    bool setRichPresenceInGame(std::string_view levelNameFormat);

    bool               setAndStoreStat(std::string_view name, int data);
    [[nodiscard]] bool getAchievement(bool* out, std::string_view name);
    [[nodiscard]] bool getStat(int* out, std::string_view name);

    bool updateHardcodedAchievements();
};

} // namespace hg::Steam
