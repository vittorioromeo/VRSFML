#pragma once


////////////////////////////////////////////////////////////
namespace sf::base
{
class StringView;
}


////////////////////////////////////////////////////////////
struct Profile;
struct Playthrough;
struct GameConstants;

////////////////////////////////////////////////////////////
bool saveProfileToFile(const Profile& profile, const char* filename = "userdata/profile.json");
bool loadProfileFromFile(Profile& profile, const char* filename = "userdata/profile.json");
bool saveGameConstantsToFile(const GameConstants& gameConstants, const char* filename = "resources/game_constants.json");
bool loadGameConstantsFromFile(GameConstants& gameConstants, const char* filename = "resources/game_constants.json");
bool savePlaythroughToFile(const Playthrough& playthrough, const char* filename);
[[nodiscard]] sf::base::StringView loadPlaythroughFromFile(Playthrough& playthrough, const char* filename);
