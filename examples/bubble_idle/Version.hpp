#pragma once


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_DEMO
    #define BUBBLEBYTE_DEMO_STR " (DEMO)"
inline constexpr bool isDemoVersion = true;
#else
    #define BUBBLEBYTE_DEMO_STR ""
inline constexpr bool isDemoVersion = false;
#endif

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY_IMPL(...) #__VA_ARGS__

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY(...) BUBBLEBYTE_STRINGIFY_IMPL(__VA_ARGS__)

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MAJOR 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MINOR 8

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_PATCH 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_STR                                                   \
    "v" BUBBLEBYTE_STRINGIFY(BUBBLEBYTE_VERSION_MAJOR) "." BUBBLEBYTE_STRINGIFY( \
        BUBBLEBYTE_VERSION_MINOR) "." BUBBLEBYTE_STRINGIFY(BUBBLEBYTE_VERSION_PATCH) BUBBLEBYTE_DEMO_STR

////////////////////////////////////////////////////////////
struct [[nodiscard]] Version
{
    unsigned int major;
    unsigned int minor;
    unsigned int patch;

    constexpr bool operator==(const Version& other) const = default;
};

////////////////////////////////////////////////////////////
inline constexpr Version currentVersion{BUBBLEBYTE_VERSION_MAJOR, BUBBLEBYTE_VERSION_MINOR, BUBBLEBYTE_VERSION_PATCH};
