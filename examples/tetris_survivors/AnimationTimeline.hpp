#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/InPlaceVector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename TCommand>
struct [[nodiscard]] AnimationTimeline
{
    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<TCommand, 128> commands;
    float                                  timeOnCurrentCommand = 0.f;


    ////////////////////////////////////////////////////////////
    void add(const float duration, const auto& cmd)
    {
        commands.emplaceBack(typename TCommand::VariantType{cmd}, duration);
    }


    ////////////////////////////////////////////////////////////
    void addInstantaneous(const auto& cmd)
    {
        commands.emplaceBack(typename TCommand::VariantType{cmd}, 0.f);
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
        return commands.empty() ? nullptr : commands.front().data.template getIf<T>();
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] const T* getIfPlaying() const
    {
        return commands.empty() ? nullptr : commands.front().data.template getIf<T>();
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool isPlaying() const
    {
        return !commands.empty() && commands.front().data.template is<T>();
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool isEnqueued() const
    {
        for (const auto& cmd : commands)
            if (cmd.data.template is<T>())
                return true;

        return false;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getProgress() const
    {
        SFML_BASE_ASSERT(!commands.empty());

        const float duration = commands.front().duration;

        if (duration <= 0.f)
            return 1.f;

        return sf::base::clamp(timeOnCurrentCommand / duration, 0.f, 1.f);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool justStarted() const
    {
        return timeOnCurrentCommand == 0.f;
    }
};

} // namespace tsurv
