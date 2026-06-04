#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Trait/IsSame.hpp"
#include "ZancleBase/Trait/RemoveCVRef.hpp"
#include "ZancleBase/Trait/RemoveReference.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
/// \brief Non-owning, lightweight reference to a callable
///
////////////////////////////////////////////////////////////
template <typename TSignature>
class FunctionRef;


////////////////////////////////////////////////////////////
/// \brief Non-owning, lightweight reference to a callable
///
/// `FunctionRef` is a simplified alternative to `std::function_ref`
/// intended for use as a function parameter, to pass any callable
/// (free function, lambda, functor) without type-erasure allocation
/// and without copying the callable.
///
/// The referenced callable must outlive the `FunctionRef`. In typical
/// use as a function parameter this is automatically the case because
/// the caller's argument lives through the full-expression of the call.
///
////////////////////////////////////////////////////////////
template <typename TReturn, typename... Ts>
class FunctionRef<TReturn(Ts...)>
{
private:
    ////////////////////////////////////////////////////////////
    using FnPtrType = TReturn (*)(Ts...);
    using ThunkType = TReturn (*)(void*, Ts&&...);


    ////////////////////////////////////////////////////////////
    void*     m_obj;
    ThunkType m_thunk;

public:
    ////////////////////////////////////////////////////////////
    FunctionRef() = delete;


    ////////////////////////////////////////////////////////////
    /// \brief Construct from a function pointer (must be non-null)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] FunctionRef(FnPtrType fn) noexcept :
        m_obj{reinterpret_cast<void*>(fn)},
        m_thunk{[](void* o, Ts&&... args) -> TReturn
    { return reinterpret_cast<FnPtrType>(o)(ZB_FORWARD(args)...); }}
    {
        ZB_ASSERT(fn != nullptr);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct from any callable object (lambda, functor, ...)
    ///
    /// The callable is referenced, not copied. Its lifetime must
    /// extend across any subsequent calls to `operator()`.
    ///
    ////////////////////////////////////////////////////////////
    template <typename TFFwd>
        requires(!base::isSame<base::RemoveCVRefIndirect<TFFwd>, FunctionRef> &&
                 !base::isSame<base::RemoveCVRefIndirect<TFFwd>, FnPtrType>)
    [[nodiscard, gnu::always_inline]] FunctionRef(TFFwd&& f) noexcept :
        m_obj{const_cast<void*>(static_cast<const void*>(&f))},
        m_thunk{[](void* o, Ts&&... args) -> TReturn
    {
        using UnrefType = ZB_REMOVE_REFERENCE(TFFwd);
        return (*reinterpret_cast<UnrefType*>(o))(ZB_FORWARD(args)...);
    }}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Invoke the referenced callable
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] TReturn operator()(Ts... args) const
    {
        return m_thunk(m_obj, ZB_FORWARD(args)...);
    }
};

} // namespace zb


////////////////////////////////////////////////////////////
/// \class zb::FunctionRef
/// \ingroup system
///
/// Non-owning, lightweight reference to a callable, intended for
/// use as a function parameter. Does not allocate, does not copy
/// the referenced callable.
///
////////////////////////////////////////////////////////////
