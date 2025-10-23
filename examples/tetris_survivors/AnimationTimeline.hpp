#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "AnimationCommands.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimationTimeline
{
    ////////////////////////////////////////////////////////////
    sf::base::Vector<AnimationCommand> commands;
    float                              timeOnCurrentCommand = 0.f;


    ////////////////////////////////////////////////////////////
    void add(const auto& cmd)
    {
        commands.emplaceBack(cmd);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyAnimationPlaying() const
    {
        return !commands.empty();
    }


    ////////////////////////////////////////////////////////////
    void popFrontCommand()
    {
        SFML_BASE_ASSERT(!commands.empty());

        commands.erase(commands.begin());
        timeOnCurrentCommand = 0.f;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] T* getIfPlaying()
    {
        return commands.empty() ? nullptr : commands.front().getIf<T>();
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool isPlaying() const
    {
        return !commands.empty() && commands.front().is<T>();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getProgress() const
    {
        if (commands.empty())
            return 0.f;

        const float duration = commands.front().linearMatch([](auto&& cmd) { return cmd.duration; });
        return sf::base::clamp(timeOnCurrentCommand / duration, 0.f, 1.f);
    }
};

} // namespace tsurv
