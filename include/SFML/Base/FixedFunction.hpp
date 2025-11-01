#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MaxAlignT.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsRvalueReference.hpp"
#include "SFML/Base/Trait/RemoveReference.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Non-allocating `std::function` alternative with fixed storage size
///
////////////////////////////////////////////////////////////
template <typename TSignature, SizeT TStorageSize>
class FixedFunction;


////////////////////////////////////////////////////////////
/// \brief Non-allocating `std::function` alternative with fixed storage size
///
////////////////////////////////////////////////////////////
template <typename TReturn, typename... Ts, SizeT TStorageSize>
class FixedFunction<TReturn(Ts...), TStorageSize>
{
private:
    ////////////////////////////////////////////////////////////
    enum class Operation : unsigned char
    {
        Destroy       = 0u,
        MoveConstruct = 1u,
        CopyConstruct = 2u,
    };


    ////////////////////////////////////////////////////////////
    using RetType = TReturn;


    ////////////////////////////////////////////////////////////
    using FnPtrType  = RetType (*)(Ts...);
    using MethodType = RetType (*)(char*, FnPtrType, Ts...);
    using AllocType  = void (*)(char*, void* objectPtr, const Operation operation);


    ////////////////////////////////////////////////////////////
    union
    {
        alignas(MaxAlignT) char objStorage[TStorageSize];
        FnPtrType functionPtr;
    };


    ////////////////////////////////////////////////////////////
    MethodType m_methodPtr;
    AllocType  m_allocPtr;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void destroyIfNeeded() noexcept
    {
        if (m_allocPtr == nullptr)
            return;

        m_allocPtr(objStorage, nullptr, Operation::Destroy);
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] FixedFunction() noexcept : functionPtr{nullptr}, m_methodPtr{nullptr}, m_allocPtr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief FixedFunction Constructor from functional object.
    ///
    /// \param f Functor object will be stored in the internal objStorage
    /// using move constructor. Unmovable objects are prohibited explicitly.
    ///
    ////////////////////////////////////////////////////////////
    template <typename TFFwd>
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    [[nodiscard]] FixedFunction(TFFwd&& f) : FixedFunction()
    {
        using UnrefType = SFML_BASE_REMOVE_REFERENCE(TFFwd);
        static_assert(sizeof(UnrefType) <= TStorageSize);

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_methodPtr = [](char* s, FnPtrType, Ts... xs) -> RetType
        { return reinterpret_cast<UnrefType*>(s)->operator()(SFML_BASE_FORWARD(xs)...); };

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_allocPtr = [](char* s, void* o, const Operation operation)
        {
            if (operation == Operation::Destroy)
            {
                SFML_BASE_ASSERT(s != nullptr);
                reinterpret_cast<UnrefType*>(s)->~UnrefType();
            }
            else if (operation == Operation::MoveConstruct)
            {
                SFML_BASE_ASSERT(o != nullptr);
                SFML_BASE_PLACEMENT_NEW(s) UnrefType(SFML_BASE_MOVE(*static_cast<UnrefType*>(o)));
            }
            else
            {
                SFML_BASE_ASSERT(operation == Operation::CopyConstruct);

                SFML_BASE_ASSERT(o != nullptr);
                SFML_BASE_PLACEMENT_NEW(s) UnrefType(*static_cast<const UnrefType*>(o));
            }
        };

        m_allocPtr(objStorage,
                   &f,
                   SFML_BASE_IS_RVALUE_REFERENCE(TFFwd&&) ? Operation::MoveConstruct : Operation::CopyConstruct);
    }


    ////////////////////////////////////////////////////////////
    template <typename TFReturn, typename... TFs>
    [[nodiscard]] FixedFunction(TFReturn (*f)(TFs...)) noexcept :
        functionPtr{f},
        m_methodPtr{[](char* /* unused */, FnPtrType xf, Ts... xs) -> RetType { return xf(SFML_BASE_FORWARD(xs)...); }},
        m_allocPtr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit FixedFunction(decltype(nullptr)) noexcept :
        functionPtr{nullptr},
        m_methodPtr{nullptr},
        m_allocPtr{nullptr}
    {
    }

    ////////////////////////////////////////////////////////////
    FixedFunction(const FixedFunction& rhs) : FixedFunction()
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(const FixedFunction& rhs)
    {
        if (this == &rhs)
            return *this;

        destroyIfNeeded();

        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return *this;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);

        return *this;
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(decltype(nullptr)) noexcept
    {
        destroyIfNeeded();

        m_methodPtr = nullptr;
        m_allocPtr  = nullptr;
        functionPtr = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    FixedFunction(FixedFunction&& rhs) noexcept : FixedFunction()
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);

        rhs.m_methodPtr = nullptr;
        rhs.m_allocPtr  = nullptr;
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(FixedFunction&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        destroyIfNeeded();

        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return *this;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);

        rhs.m_methodPtr = nullptr;
        rhs.m_allocPtr  = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~FixedFunction() noexcept
    {
        destroyIfNeeded();
    }


    ////////////////////////////////////////////////////////////
    template <typename... TArgs>
    [[gnu::always_inline, gnu::flatten]] RetType operator()(TArgs&&... args)
    {
        SFML_BASE_ASSERT(m_methodPtr != nullptr);
        return m_methodPtr(objStorage, functionPtr, SFML_BASE_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit operator bool() const
    {
        return m_methodPtr != nullptr;
    }
};

} // namespace sf::base

////////////////////////////////////////////////////////////
/// \class sf::base::FixedFunction
/// \ingroup system
///
/// Non-allocating `std::function` alternative with fixed storage size
///
////////////////////////////////////////////////////////////
