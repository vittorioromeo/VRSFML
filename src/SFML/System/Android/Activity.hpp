#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/EglContext.hpp"
#include "SFML/Window/Event.hpp"

#include "SFML/Base/EnumArray.hpp"

#include <android/configuration.h>
#include <android/native_activity.h>

#include <mutex>
#include <streambuf>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstddef>


class SFML_SYSTEM_API LogcatStream : public std::streambuf
{
public:
    LogcatStream() = default;

    std::streambuf::int_type overflow(std::streambuf::int_type c) override;

private:
    std::string m_message;
};

namespace sf::priv
{
struct ActivityStates
{
    ANativeActivity* activity{};
    ANativeWindow*   window{};

    ALooper*        looper{};
    AInputQueue*    inputQueue{};
    AConfiguration* config{};

    EGLDisplay  display{};
    EglContext* context{};

    std::vector<std::byte> savedState;

    std::recursive_mutex mutex;

    void (*forwardEvent)(const Event& event){};
    int (*processEvent)(int fd, int events, void* data){};

    std::unordered_map<int, Vector2i>                  touchEvents;
    Vector2i                                           mousePosition;
    EnumArray<Mouse::Button, bool, Mouse::ButtonCount> isButtonPressed{};

    bool mainOver{};

    Vector2i screenSize;

    bool initialized{};
    bool terminated{};

    bool fullscreen{};

    bool updated{};

    LogcatStream logcat;
};

SFML_SYSTEM_API ActivityStates*& getActivityStatesPtr();

SFML_SYSTEM_API void resetActivity(ActivityStates* initializedStates);

SFML_SYSTEM_API ActivityStates& getActivity();

} // namespace sf::priv
