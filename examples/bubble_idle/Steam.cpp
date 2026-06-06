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

#include "Zancle/Err/FmtPath.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"

#include "Zancle/IO/IO.hpp"

#include "Zancle/String/StringView.hpp"

#include "Zancle/Container/AnkerlUnorderedDense.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Diagnostic/Assert.hpp"

#include "Zancle/Base/SizeT.hpp"

#include <cstdio>
#include <cstring>


namespace hg::Steam
{

namespace
{
[[nodiscard]] za::Optional<CSteamID> getUserSteamId()
{
    // Using C API here because C++ one doesn't work with MinGW.

    ISteamUser* steamUser = SteamAPI_SteamUser_v023();

    if (!SteamAPI_ISteamUser_BLoggedOn(steamUser))
    {
        za::printLn("[Steam]: Attempted to retrieve Steam ID when not logged in");

        return za::nullOpt;
    }

    return za::makeOptional(CSteamID{SteamAPI_ISteamUser_GetSteamID(steamUser)});
}

[[nodiscard]] bool initializeSteamworks()
{
    za::printLn("[Steam]: Initializing Steam API");

    SteamErrMsg errMsg;
    if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
    {
        za::printLn("[Steam]: Failed to initialize Steam API: {}", errMsg);
        return false;
    }

    za::printLn("[Steam]: Steam API successfully initialized");

    if (const za::Optional<CSteamID> userSteamId = getUserSteamId(); userSteamId.hasValue())
    {
        za::printLn("[Steam]: User Steam ID: '{}'", userSteamId->ConvertToUint64());
    }
    else
    {
        za::printLn("[Steam]: Could not retrieve user Steam ID");
    }

    return true;
}

void shutdownSteamworks()
{
    za::printLn("[Steam]: Shutting down Steam API");
    SteamAPI_Shutdown();
    za::printLn("[Steam]: Shut down Steam API");
}

} // namespace

class SteamManager::SteamManagerImpl
{
public:
    bool m_initialized;
    bool m_gotStats;

    ankerl::unordered_dense::set<za::SizeT> m_unlockedAchievements;

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
    bool unlockAchievement(za::SizeT idx);
    bool isAchievementUnlocked(za::SizeT idx);

    bool setRichPresenceInGame(za::StringView levelNameFormat);

    bool                             setAndStoreStat(za::StringView name, int data);
    [[nodiscard]] bool               getAchievement(bool* out, za::StringView name);
    [[nodiscard]] bool               getStat(int* out, za::StringView name);
    [[nodiscard]] za::Optional<bool> isAchievementUnlocked(const char* name);

    bool updateHardcodedAchievements();
};

void SteamManager::SteamManagerImpl::onUserStatsReceived(UserStatsReceived_t* data)
{
    (void)data;

    za::printLn("[Steam]: Received user stats (rc: {})", data->m_eResult);

    m_gotStats = true;
}

void SteamManager::SteamManagerImpl::onUserStatsStored(UserStatsStored_t* data)
{
    (void)data;

    za::printLn("[Steam]: Stored user stats");
}

void SteamManager::SteamManagerImpl::onUserAchievementStored(UserAchievementStored_t* data)
{
    (void)data;

    za::printLn("[Steam]: Stored user achievement");
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
        za::printLn("[Steam]: Attempted to request stats when uninitialized");
        return false;
    }

    static thread_local za::Optional<CSteamID> cachedUserSteamId;

    if (!cachedUserSteamId.hasValue())
    {
        cachedUserSteamId = getUserSteamId();

        if (!cachedUserSteamId.hasValue())
            return false;

        za::printLn("[Steam]: Cached User Steam ID: '{}'", cachedUserSteamId->ConvertToUint64());
    }

    if (!SteamUserStats()->RequestUserStats(cachedUserSteamId.value()))
    {
        za::printLn("[Steam]: Failed to get stats and achievements");
        m_gotStats = false;
        return false;
    }

    za::printLn("[Steam]: Successfully requested stats and achievements");
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
        za::printLn("[Steam]: Attempted to store stats when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        za::printLn("[Steam]: Attempted to store stat without stats");
        return false;
    }

    if (!SteamUserStats()->StoreStats())
    {
        za::printLn("[Steam]: Failed to store stats");
        return false;
    }

    return true;
}

bool SteamManager::SteamManagerImpl::unlockAchievement(za::SizeT idx)
{
    if (!m_initialized)
    {
        za::printLn("[Steam]: Attempted to unlock achievement when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        za::printLn("[Steam]: Attempted to unlock achievement without stats");
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
        za::printLn("[Steam]: Failed to unlock achievement {}", buf);
        return false;
    }

    m_unlockedAchievements.emplace(idx);
    return storeStats();
}

bool SteamManager::SteamManagerImpl::isAchievementUnlocked(za::SizeT idx)
{
    if (!m_initialized)
    {
        za::printLn("[Steam]: Attempted to check achievement when uninitialized");
        return false;
    }

    if (!m_gotStats)
    {
        za::printLn("[Steam]: Attempted to check achievement without stats");
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

bool SteamManager::SteamManagerImpl::setRichPresenceInGame(za::StringView levelNameFormat)
{
    if (!m_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence("levelname", levelNameFormat.data()) &&
           SteamFriends()->SetRichPresence("steam_display", "#InGame");
}

bool SteamManager::SteamManagerImpl::setAndStoreStat(za::StringView name, int data)
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
        za::printLn("[Steam]: Error setting stat '{}' to '{}'", name, asFloat);

        return false;
    }

    return storeStats();
}

[[nodiscard]] bool SteamManager::SteamManagerImpl::getAchievement(bool* out, za::StringView name)
{
    if (!m_initialized || !m_gotStats)
    {
        return false;
    }

    if (!SteamUserStats()->GetAchievement(name.data(), out))
    {
        za::printLn("[Steam]: Error getting achievement {}", name);
        return false;
    }

    return true;
}

[[nodiscard]] bool SteamManager::SteamManagerImpl::getStat(int* out, za::StringView name)
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

    za::printLn("[Steam]: Error getting stat {}", name.data());
    return false;
}

[[nodiscard]] za::Optional<bool> SteamManager::SteamManagerImpl::isAchievementUnlocked(const char* name)
{
    bool       res{false};
    const bool rc = getAchievement(&res, name);

    if (!rc)
    {
        return za::nullOpt;
    }

    return za::makeOptional(res);
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
    ZA_ASSERT(m_impl != nullptr);
    return *m_impl;
}

[[nodiscard]] SteamManager::SteamManagerImpl& SteamManager::impl() noexcept
{
    ZA_ASSERT(m_impl != nullptr);
    return *m_impl;
}

SteamManager::SteamManager() : m_impl{za::makeUnique<SteamManagerImpl>()}
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

bool SteamManager::unlockAchievement(za::SizeT idx)
{
    return impl().unlockAchievement(idx);
}

bool SteamManager::isAchievementUnlocked(za::SizeT idx)
{
    return impl().isAchievementUnlocked(idx);
}

bool SteamManager::setRichPresenceInGame(za::StringView levelNameFormat)
{
    return impl().setRichPresenceInGame(levelNameFormat);
}

bool SteamManager::setAndStoreStat(za::StringView name, int data)
{
    return impl().setAndStoreStat(name, data);
}

[[nodiscard]] bool SteamManager::getAchievement(bool* out, za::StringView name)
{
    return impl().getAchievement(out, name);
}

[[nodiscard]] bool SteamManager::getStat(int* out, za::StringView name)
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
