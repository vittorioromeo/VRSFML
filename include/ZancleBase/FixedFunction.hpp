#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Launder.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/MaxAlignT.hpp"
#include "ZancleBase/PlacementNew.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Trait/IsRvalueReference.hpp"
#include "ZancleBase/Trait/IsSame.hpp"
#include "ZancleBase/Trait/RemoveCVRef.hpp"
#include "ZancleBase/Trait/RemoveReference.hpp"

// TODO P1: provide triviallyrelocatable version

namespace zb
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
        requires(!zb::isSame<zb::RemoveCVRefIndirect<TFFwd>, FixedFunction>)
    [[nodiscard]] FixedFunction(TFFwd&& f) : FixedFunction()
    {
        using UnrefType = ZB_REMOVE_REFERENCE(TFFwd);

        static_assert(sizeof(UnrefType) <= TStorageSize);
        static_assert(alignof(UnrefType) <= alignof(MaxAlignT));

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_methodPtr = [](char* s, FnPtrType, Ts... xs) -> RetType
        { return ZB_LAUNDER_CAST(UnrefType*, s)->operator()(ZB_FORWARD(xs)...); };

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_allocPtr = [](char* s, void* o, const Operation operation)
        {
            if (operation == Operation::Destroy)
            {
                ZB_ASSERT(s != nullptr);
                ZB_LAUNDER_CAST(UnrefType*, s)->~UnrefType();
            }
            else if (operation == Operation::MoveConstruct)
            {
                ZB_ASSERT(o != nullptr);
                ZB_PLACEMENT_NEW(s) UnrefType(ZB_MOVE(*static_cast<UnrefType*>(o)));
            }
            else
            {
                ZB_ASSERT(operation == Operation::CopyConstruct);

                ZB_ASSERT(o != nullptr);
                ZB_PLACEMENT_NEW(s) UnrefType(*static_cast<const UnrefType*>(o));
            }
        };

        m_allocPtr(objStorage,
                   &f,
                   ZB_IS_RVALUE_REFERENCE(TFFwd&&) ? Operation::MoveConstruct : Operation::CopyConstruct);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] FixedFunction(FnPtrType f) noexcept :
        functionPtr{f},
        m_methodPtr{[](char* /* unused */, FnPtrType xf, Ts... xs) -> RetType { return xf(ZB_FORWARD(xs)...); }},
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
        return const_cast<const FixedFunction&>(*this)(ZB_FORWARD(args)...);
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
        ZB_ASSERT(m_methodPtr != nullptr);
        return m_methodPtr(const_cast<char*>(objStorage), functionPtr, ZB_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit operator bool() const
    {
        return m_methodPtr != nullptr;
    }
};

} // namespace zb


////////////////////////////////////////////////////////////
/// \class zb::FixedFunction
/// \ingroup system
///
/// Non-allocating `std::function` alternative with fixed storage size
///
////////////////////////////////////////////////////////////
