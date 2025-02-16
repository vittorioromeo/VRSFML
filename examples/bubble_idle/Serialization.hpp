#pragma once


////////////////////////////////////////////////////////////
struct Profile;
struct Playthrough;

////////////////////////////////////////////////////////////
void saveProfileToFile(const Profile& profile, const char* filename = "userdata/profile.json");
void loadProfileFromFile(Profile& profile, const char* filename = "userdata/profile.json");
void savePlaythroughToFile(const Playthrough& playthrough, const char* filename = "userdata/playthrough.json");
void loadPlaythroughFromFile(Playthrough& playthrough, const char* filename = "userdata/playthrough.json");
