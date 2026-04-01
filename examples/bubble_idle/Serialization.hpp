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
void saveProfileToFile(const Profile& profile, const char* filename = "userdata/profile.json");
void loadProfileFromFile(Profile& profile, const char* filename = "userdata/profile.json");
void saveGameConstantsToFile(const GameConstants& gameConstants, const char* filename = "resources/game_constants.json");
void loadGameConstantsFromFile(GameConstants& gameConstants, const char* filename = "resources/game_constants.json");
void savePlaythroughToFile(const Playthrough& playthrough, const char* filename);
[[nodiscard]] sf::base::StringView loadPlaythroughFromFile(Playthrough& playthrough, const char* filename);
