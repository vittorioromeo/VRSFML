#pragma once


////////////////////////////////////////////////////////////
namespace sf::base
{
class StringView;
}


////////////////////////////////////////////////////////////
struct Profile;
struct Playthrough;

////////////////////////////////////////////////////////////
void saveProfileToFile(const Profile& profile, const char* filename = "userdata/profile.json");
void loadProfileFromFile(Profile& profile, const char* filename = "userdata/profile.json");
void savePlaythroughToFile(const Playthrough& playthrough, const char* filename);
[[nodiscard]] sf::base::StringView loadPlaythroughFromFile(Playthrough& playthrough, const char* filename);
