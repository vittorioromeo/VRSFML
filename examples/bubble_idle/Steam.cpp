// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "Steam.hpp"

//
#include <stdint.h> // Steam API needs this.
//
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#include "steam/steamencryptedappticket.h"
//

#include "SFML/System/Fmt/FmtPath.hpp"
#include "SFML/System/IO.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <cstdio>
#include <cstring>


namespace hg::Steam
{

namespace
{
[[nodiscard]] sf::base::Optional<CSteamID> getUserSteamId()
{
    // Using C API here because C++ one doesn't work with MinGW.

    ISteamUser* steamUser = SteamAPI_SteamUser_v023();

    if (!SteamAPI_ISteamUser_BLoggedOn(steamUser))
    {
        sf::base::printLn("[Steam]: Attempted to retrieve Steam ID when not logged in");

        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(CSteamID{SteamAPI_ISteamUser_GetSteamID(steamUser)});
}

[[nodiscard]] bool initializeSteamworks()
{
    sf::base::printLn("[Steam]: Initializing Steam API");

    SteamErrMsg errMsg;
    if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
    {
        sf::base::printLn("[Steam]: Failed to initialize Steam API: {}", errMsg);
        return false;
    }

    sf::base::printLn("[Steam]: Steam API successfully initialized");

    if (const sf::base::Optional<CSteamID> userSteamId = getUserSteamId(); userSteamId.hasValue())
    {
        sf::base::printLn("[Steam]: User Steam ID: '{}'", userSteamId->ConvertToUint64());
    }
    else
    {
        sf::base::printLn("[Steam]: Could not retrieve user Steam ID");
    }

    return true;
}

void shutdownSteamworks()
{
    sf::base::printLn("[Steam]: Shutting down Steam API");
    SteamAPI_Shutdown();
    sf::base::printLn("[Steam]: Shut down Steam API");
}

} // namespace

class SteamManager::SteamManagerImpl
{
public:
    bool m_initialized;
    bool m_gotStats;

    ankerl::unordered_dense::set<sf::base::SizeT> m_unlockedAchievements;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#if defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
    STEAM_CALLBACK(SteamManagerImpl, onUserStatsReceived, UserStatsReceived_t);
    STEAM_CALLBACK(SteamManagerImpl, onUserStatsStored, UserStatsStored_t);
    STEAM_CALLBACK(SteamManagerImpl, onUserAchievementStored, UserAchievementStored_t);
#if defined(__clang__)
    #pragma GCC diagnostic pop
#endif
#pragma GCC diagnostic pop

    bool updateHardcodedAchievementCubeMaster();


public:
    SteamManagerImpl();
    ~SteamManagerImpl();

    SteamManagerImpl(const SteamManagerImpl&)            = delete;
    SteamManagerImpl& operator=(const SteamManagerImpl&) = delete;

    SteamManagerImpl(SteamManagerImpl&&)            = delete;
    SteamManagerImpl& operator=(SteamManagerImpl&&) = delete;

    [[nodiscard]] bool isInitialized() const noexcept;

    bool requestStatsAndAchievements();

    bool runCallbacks();

    bool storeStats();
    bool unlockAchievement(sf::base::SizeT idx);
    bool isAchievementUnlocked(sf::base::SizeT idx);

    bool setRichPresenceInGame(sf::base::StringView levelNameFormat);

    bool                                   setAndStoreStat(sf::base::StringView name, int data);
    [[nodiscard]] bool                     getAchievement(bool* out, sf::base::StringView name);
    [[nodiscard]] bool                     getStat(int* out, sf::base::StringView name);
    [[nodiscard]] sf::base::Optional<bool> isAchievementUnlocked(const char* name);

    bool updateHardcodedAchievements();
};

void SteamManager::SteamManagerImpl::onUserStatsReceived(UserStatsReceived_t* data)
{
    (void)data;

    sf::base::printLn("[Steam]: Received user stats (rc: {})", data->m_eResult);

    m_gotStats = true;
}

void SteamManager::SteamManagerImpl::onUserStatsStored(UserStatsStored_t* data)
{
    (void)data;

    sf::base::printLn("[Steam]: Stored user stats");
}

void SteamManager::SteamManagerImpl::onUserAchievementStored(UserAchievementStored_t* data)
{
    (void)data;

    sf::base::printLn("[Steam]: Stored user achievement");
}

SteamManager::SteamManagerImpl::SteamManagerImpl() : m_initialized{initializeSteamworks()}, m_gotStats{false}
{
    if (!m_initialized)
    {
        return;
    }
}

SteamManager::SteamManagerImpl::~SteamManagerImpl()
{
    if (m_initialized)
    {
        shutdownSteamworks();
    }
}

[[nodiscard]] bool SteamManager::SteamManagerImpl::isInitialized() const noexcept
{
    return m_initialized;
}

bool SteamManager::SteamManagerImpl::requestStatsAndAchievements()
{
    if (!m_initialized)
    {
        sf::base::printLn("[Steam]: Attempted to request stats when uninitialized");
        return false;
    }

    static thread_local sf::base::Optional<CSteamID> cachedUserSteamId;

    if (!cachedUserSteamId.hasValue())
    {
        cachedUserSteamId = getUserSteamId();

        if (!cachedUserSteamId.hasValue())
            return false;

        sf::base::printLn("[Steam]: Cached User Steam ID: '{}'", cachedUserSteamId->ConvertToUint64());
    }

    if (!SteamUserStats()->RequestUserStats(cachedUserSteamId.value()))
    {
        sf::base::printLn("[Steam]: Failed to get stats and achievements");
        m_gotStats = false;
        return false;
    }

    sf::base::printLn("[Steam]: Successfully requested stats and achievements");
    return true;
}

bool SteamManager::SteamManagerImpl::runCallbacks()
{
    if (!m_initialized)
    {
        return false;
    }

    SteamAPI_RunCallbacks();
    return true;
}

bool SteamManager::SteamManagerImpl::storeStats()
{
    if (!m_initialized)
    {
        sf::base::printLn("[Steam]: Attempted to store stats when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        sf::base::printLn("[Steam]: Attempted to store stat without stats");
        return false;
    }

    if (!SteamUserStats()->StoreStats())
    {
        sf::base::printLn("[Steam]: Failed to store stats");
        return false;
    }

    return true;
}

bool SteamManager::SteamManagerImpl::unlockAchievement(sf::base::SizeT idx)
{
    if (!m_initialized)
    {
        sf::base::printLn("[Steam]: Attempted to unlock achievement when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        sf::base::printLn("[Steam]: Attempted to unlock achievement without stats");
        return false;
    }

    if (m_unlockedAchievements.contains(idx))
    {
        return false;
    }

    char buf[64];
    std::snprintf(buf, sizeof(buf), "ACH_%zu", idx);

    if (!SteamUserStats()->SetAchievement(buf))
    {
        sf::base::printLn("[Steam]: Failed to unlock achievement {}", buf);
        return false;
    }

    m_unlockedAchievements.emplace(idx);
    return storeStats();
}

bool SteamManager::SteamManagerImpl::isAchievementUnlocked(sf::base::SizeT idx)
{
    if (!m_initialized)
    {
        sf::base::printLn("[Steam]: Attempted to check achievement when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        sf::base::printLn("[Steam]: Attempted to check achievement without stats");
        return false;
    }

    char buf[64];
    std::snprintf(buf, sizeof(buf), "ACH_%zu", idx);

    bool unlocked = false;
    if (!getAchievement(&unlocked, buf))
    {
        return false;
    }

    return unlocked;
}

bool SteamManager::SteamManagerImpl::setRichPresenceInGame(sf::base::StringView levelNameFormat)
{
    if (!m_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence("levelname", levelNameFormat.data()) &&
           SteamFriends()->SetRichPresence("steam_display", "#InGame");
}

bool SteamManager::SteamManagerImpl::setAndStoreStat(sf::base::StringView name, int data)
{
    if (!m_initialized)
    {
        return false;
    }

    // Steam API seems to be bugged, and sometimes needs floats even for integer
    // stats.
    const auto asFloat = static_cast<float>(data);
    if (!SteamUserStats()->SetStat(name.data(), asFloat) && // Try with float.
        !SteamUserStats()->SetStat(name.data(), data))      // Try with integer.
    {
        sf::base::printLn("[Steam]: Error setting stat '{}' to '{}'", name, asFloat);

        return false;
    }

    return storeStats();
}

[[nodiscard]] bool SteamManager::SteamManagerImpl::getAchievement(bool* out, sf::base::StringView name)
{
    if (!m_initialized || !m_gotStats)
    {
        return false;
    }

    if (!SteamUserStats()->GetAchievement(name.data(), out))
    {
        sf::base::printLn("[Steam]: Error getting achievement {}", name);
        return false;
    }

    return true;
}

[[nodiscard]] bool SteamManager::SteamManagerImpl::getStat(int* out, sf::base::StringView name)
{
    if (!m_initialized || !m_gotStats)
    {
        return false;
    }

    // Steam API seems to be bugged, and sometimes needs floats even for integer
    // stats.
    float asFloat;
    if (SteamUserStats()->GetStat(name.data(), &asFloat)) // Try with float.
    {
        *out = static_cast<int>(asFloat);
        return true;
    }

    if (SteamUserStats()->GetStat(name.data(), out)) // Try with integer.
    {
        return true;
    }

    sf::base::printLn("[Steam]: Error getting stat {}", name.data());
    return false;
}

[[nodiscard]] sf::base::Optional<bool> SteamManager::SteamManagerImpl::isAchievementUnlocked(const char* name)
{
    bool       res{false};
    const bool rc = getAchievement(&res, name);

    if (!rc)
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(res);
}

bool SteamManager::SteamManagerImpl::updateHardcodedAchievementCubeMaster()
{
    if (!m_initialized || !m_gotStats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int { return isAchievementUnlocked(name).valueOr(false) ? 1 : 0; };

    // "Cube Master"
    {
        int        stat;
        const bool rc = getStat(&stat, "s0_packprogress_cube");

        if (!rc)
        {
            return false;
        }

        const int acc = unlocked("a0_babysteps") +  //
                        unlocked("a1_pointless") +  //
                        unlocked("a2_flattering") + //
                        unlocked("a3_seconddim") +  //
                        unlocked("a4_apeirogon") +  //
                        unlocked("a5_commando") +   //
                        unlocked("a6_euclidian") +  //
                        unlocked("a7_pi") +         //
                        unlocked("a8_lab") +        //
                        unlocked("a9_ratio");

        if (acc > stat)
        {
            if (!setAndStoreStat("s0_packprogress_cube", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool SteamManager::SteamManagerImpl::updateHardcodedAchievements()
{
    // Intentionally not short-circuiting via boolean operators here.

    int failures = 0;

    failures += static_cast<int>(!updateHardcodedAchievementCubeMaster());

    return failures == 0;
}
// ----------------------------------------------------------------------------

[[nodiscard]] const SteamManager::SteamManagerImpl& SteamManager::impl() const noexcept
{
    SFML_BASE_ASSERT(m_impl != nullptr);
    return *m_impl;
}

[[nodiscard]] SteamManager::SteamManagerImpl& SteamManager::impl() noexcept
{
    SFML_BASE_ASSERT(m_impl != nullptr);
    return *m_impl;
}

SteamManager::SteamManager() : m_impl{sf::base::makeUnique<SteamManagerImpl>()}
{
}

SteamManager::~SteamManager() = default;

[[nodiscard]] bool SteamManager::isInitialized() const noexcept
{
    return impl().isInitialized();
}

bool SteamManager::requestStatsAndAchievements()
{
    return impl().requestStatsAndAchievements();
}

bool SteamManager::runCallbacks()
{
    return impl().runCallbacks();
}

bool SteamManager::storeStats()
{
    return impl().storeStats();
}

bool SteamManager::unlockAchievement(sf::base::SizeT idx)
{
    return impl().unlockAchievement(idx);
}

bool SteamManager::isAchievementUnlocked(sf::base::SizeT idx)
{
    return impl().isAchievementUnlocked(idx);
}

bool SteamManager::setRichPresenceInGame(sf::base::StringView levelNameFormat)
{
    return impl().setRichPresenceInGame(levelNameFormat);
}

bool SteamManager::setAndStoreStat(sf::base::StringView name, int data)
{
    return impl().setAndStoreStat(name, data);
}

[[nodiscard]] bool SteamManager::getAchievement(bool* out, sf::base::StringView name)
{
    return impl().getAchievement(out, name);
}

[[nodiscard]] bool SteamManager::getStat(int* out, sf::base::StringView name)
{
    return impl().getStat(out, name);
}

bool SteamManager::updateHardcodedAchievements()
{
    return impl().updateHardcodedAchievements();
}

bool SteamManager::isOnSteamDeck()
{
    if (!impl().m_initialized)
        return false;

    return SteamUtils()->IsSteamRunningOnSteamDeck();
}

} // namespace hg::Steam
