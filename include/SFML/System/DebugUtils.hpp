////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

#ifndef SFML_DEBUG

#define SFML_DEBUG_DEFINE_DEPENDENCY_TRACKER_MEMBER(dependencyType)
#define SFML_DEBUG_DEFINE_DEPENDENT_TRACKER_MEMBER(dependencyType, dependantType, dependency)
#define SFML_DEBUG_UPDATE_DEPENDENT_TRACKER_MEMBER(dependency)

#else

#define SFML_DEBUG_DEFINE_DEPENDENCY_TRACKER_MEMBER(dependencyType) \
    friend class ::sf::priv::Dependent;                             \
    mutable ::sf::priv::Dependency m_dependency                     \
    {                                                               \
        #dependencyType                                             \
    }

#define SFML_DEBUG_DEFINE_DEPENDENT_TRACKER_MEMBER(dependencyType, dependantType, dependency) \
    ::sf::priv::Dependent m_dependent                                                         \
    {                                                                                         \
        this, &dependantType ::dependency                                                     \
    }

#define SFML_DEBUG_UPDATE_DEPENDENT_TRACKER_MEMBER(dependency) this->m_dependent.changeDependency(dependency)

#include <SFML/Config.hpp>

#include <SFML/System/Export.hpp>

#include <SFML/System/Err.hpp>

#include <exception>
#include <ostream>

#include <cassert>
#include <cstddef>

namespace sf::priv
{

template <typename T>
struct ZeroOnMove
{
    T value{};

    ZeroOnMove() = default;

    explicit ZeroOnMove(T theValue) : value{theValue}
    {
    }

    ZeroOnMove(const ZeroOnMove&)      = delete;
    ZeroOnMove& operator=(ZeroOnMove&) = delete;

    ZeroOnMove(ZeroOnMove&& rhs) noexcept : value{rhs.value}
    {
        rhs.value = T{};
    }

    ZeroOnMove& operator=(ZeroOnMove&& rhs) noexcept
    {
        if (this != &rhs)
        {
            value     = rhs.value;
            rhs.value = T{};
        }

        return *this;
    }
};

class Dependency
{
private:
    const char*             m_name;
    ZeroOnMove<std::size_t> m_dependantCount;

public:
    explicit Dependency(const char* name) : m_name{name}
    {
    }

    Dependency(const Dependency& rhs) : m_name{rhs.m_name}, m_dependantCount{0}
    {
    }

    Dependency& operator=(const Dependency& rhs)
    {
        if (this != &rhs)
        {
            m_name                 = rhs.m_name;
            m_dependantCount.value = 0;
        }

        return *this;
    }

    Dependency(Dependency&& rhs)            = default;
    Dependency& operator=(Dependency&& rhs) = default;

    ~Dependency()
    {
        if (m_dependantCount.value == 0)
            return;

        err() << "FATAL ERROR: a depedency of type '" << m_name << "' has gone out of scope before its '"
              << m_dependantCount.value
              << "' dependants.\nThis situation results in undefined behavior and memory corruption, and is likely "
                 "caused by creating the dependency as a local variable and taking its address in a dependant "
                 "object "
                 "that lives longer than the dependency itself.\nPlease make sure that your lifetime hierarchy "
                 "ensures "
                 "that dependencies will outlive their dependants.\nSee TODO:<URL> for common mistakes and how to "
                 "resolve them."
              << std::endl;

        std::terminate();
    }

    void registerDependant()
    {
        ++m_dependantCount.value;
    }

    void unregisterDependant()
    {
        if (m_dependantCount.value == 0)
        {
            throw 100;
        }
        assert(m_dependantCount.value > 0);
        --m_dependantCount.value;
    }

    const char* getName() const
    {
        return m_name;
    }
};

// TODO: something such as `Sprite::setTexture` or `Text::setFont` doesn't work, need to update internal dependencyPtr somehow
class Dependent
{
private:
    ZeroOnMove<Dependency*> m_dependencyPtr;

    void registerSelf() const
    {
        assert(m_dependencyPtr.value != nullptr);
        m_dependencyPtr.value->registerDependant();
    }

    void unregisterSelf() const
    {
        assert(m_dependencyPtr.value != nullptr);
        m_dependencyPtr.value->unregisterDependant();
    }

public:
    template <typename TDependent, typename TDependency>
    explicit Dependent(TDependent* dependent, const TDependency* TDependent::*pmr) :
    m_dependencyPtr{&(dependent->*pmr)->m_dependency}
    {
        registerSelf();
    }

    Dependent(const Dependent& rhs) : m_dependencyPtr{rhs.m_dependencyPtr.value}
    {
        registerSelf();
    }

    Dependent& operator=(const Dependent& rhs)
    {
        if (this != &rhs)
        {
            m_dependencyPtr.value = rhs.m_dependencyPtr.value;
            registerSelf();
        }

        return *this;
    }

    Dependent(Dependent&& rhs)            = default;
    Dependent& operator=(Dependent&& rhs) = default;

    ~Dependent()
    {
        if (m_dependencyPtr.value != nullptr)
            unregisterSelf();
    }

    template <typename TDependency>
    void changeDependency(const TDependency* dependency)
    {
        if (m_dependencyPtr.value != nullptr)
            unregisterSelf();

        m_dependencyPtr.value = &dependency->m_dependency;
        registerSelf();
    }
};

} // namespace sf::priv

#endif // SFML_DEBUG
