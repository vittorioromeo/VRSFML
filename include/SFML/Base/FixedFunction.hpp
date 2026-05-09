#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MaxAlignT.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsRvalueReference.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/Trait/RemoveReference.hpp"

// TODO P1: provide triviallyrelocatable version

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
    /// \brief Construct from a callable; stored in internal storage via move construct (rejects unmovable callables)
    ///
    ////////////////////////////////////////////////////////////
    template <typename TFFwd>
        requires(!base::isSame<base::RemoveCVRefIndirect<TFFwd>, FixedFunction>)
    [[nodiscard]] FixedFunction(TFFwd&& f) : FixedFunction()
    {
        using UnrefType = SFML_BASE_REMOVE_REFERENCE(TFFwd);

        static_assert(sizeof(UnrefType) <= TStorageSize);
        static_assert(alignof(UnrefType) <= alignof(MaxAlignT));

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_methodPtr = [](char* s, FnPtrType, Ts... xs) -> RetType
        { return SFML_BASE_LAUNDER_CAST(UnrefType*, s)->operator()(SFML_BASE_FORWARD(xs)...); };

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_allocPtr = [](char* s, void* o, const Operation operation)
        {
            if (operation == Operation::Destroy)
            {
                SFML_BASE_ASSERT(s != nullptr);
                SFML_BASE_LAUNDER_CAST(UnrefType*, s)->~UnrefType();
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
    [[nodiscard]] FixedFunction(FnPtrType f) noexcept :
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
        if (rhs.m_allocPtr == nullptr)
        {
            // Free-function path: no copy can throw, set both fields.
            m_methodPtr = rhs.m_methodPtr;
            functionPtr = rhs.functionPtr;
            return;
        }

        // Stored-callable path: the copy-construct can throw.
        // Update our metadata only after it succeeds, so that a
        // partially-constructed `*this` destroys cleanly during
        // exception unwind (`m_allocPtr == nullptr` -> no-op).
        rhs.m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);
        m_methodPtr = rhs.m_methodPtr;
        m_allocPtr  = rhs.m_allocPtr;
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(const FixedFunction& rhs)
    {
        if (this == &rhs)
            return *this;

        destroyIfNeeded();

        m_allocPtr  = nullptr; // Safe empty state in case copy-construct throws
        m_methodPtr = nullptr;

        if (rhs.m_allocPtr == nullptr)
        {
            // Free-function path: no copy can throw, set both fields.
            m_methodPtr = rhs.m_methodPtr;
            functionPtr = rhs.functionPtr;
            return *this;
        }

        // Stored-callable path: the copy-construct can throw.
        // Update our metadata only after it succeeds.
        rhs.m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);
        m_methodPtr = rhs.m_methodPtr;
        m_allocPtr  = rhs.m_allocPtr;

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
            functionPtr     = rhs.functionPtr;
            rhs.m_methodPtr = nullptr;
            rhs.functionPtr = nullptr;

            return;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);

        rhs.m_allocPtr(rhs.objStorage, nullptr, Operation::Destroy);
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
        m_allocPtr  = nullptr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr     = rhs.functionPtr;
            rhs.m_methodPtr = nullptr;
            rhs.functionPtr = nullptr;

            return *this;
        }

        m_allocPtr = rhs.m_allocPtr;
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);

        rhs.m_allocPtr(rhs.objStorage, nullptr, Operation::Destroy);
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
    /// \brief Invoke the wrapped callable
    ///
    /// Delegates to the `const`-qualified overload below; the
    /// underlying trampoline always invokes the wrapped callable
    /// through a non-`const` path regardless, so providing both
    /// overloads is purely for API symmetry with `std::function`.
    ///
    ////////////////////////////////////////////////////////////
    template <typename... TArgs>
    [[gnu::always_inline, gnu::flatten]] RetType operator()(TArgs&&... args)
    {
        return const_cast<const FixedFunction&>(*this)(SFML_BASE_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Invoke the wrapped callable through a `const` reference
    ///
    /// Mirrors `std::function::operator() const`. The wrapped callable
    /// is invoked through a non-`const` path regardless of `*this`'s
    /// const-ness, so a callable whose `operator()` is non-`const`
    /// will mutate its captured state. This is a *logical* const
    /// violation. Wrap mutable closures in a non-`const`
    /// `FixedFunction` if you don't want this behavior.
    ///
    ////////////////////////////////////////////////////////////
    template <typename... TArgs>
    [[gnu::always_inline, gnu::flatten]] RetType operator()(TArgs&&... args) const
    {
        SFML_BASE_ASSERT(m_methodPtr != nullptr);
        return m_methodPtr(const_cast<char*>(objStorage), functionPtr, SFML_BASE_FORWARD(args)...);
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
