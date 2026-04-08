#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Throws (or aborts) when an empty `Optional` is accessed via `value()`
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void throwIfNotEngaged();

} // namespace sf::base::priv


namespace sf::base
{
// clang-format off
////////////////////////////////////////////////////////////
/// \brief Exception type thrown by `Optional::value()` on an empty optional
///
////////////////////////////////////////////////////////////
struct BadOptionalAccess { };


////////////////////////////////////////////////////////////
/// \brief Tag type used to request in-place construction of an `Optional`'s value
///
////////////////////////////////////////////////////////////
inline constexpr struct InPlace  { } inPlace;

////////////////////////////////////////////////////////////
/// \brief Tag type used to construct an empty `Optional` (engaged == false)
///
////////////////////////////////////////////////////////////
inline constexpr struct NullOpt  { } nullOpt;

////////////////////////////////////////////////////////////
/// \brief Tag type used to construct an `Optional` from the result of an invocable
///
////////////////////////////////////////////////////////////
inline constexpr struct FromFunc { } fromFunc;
// clang-format on


////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, engaged, buffer) \
    do                                                            \
    {                                                             \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T))    \
        {                                                         \
            if (engaged)                                          \
                buffer.obj.~T();                                  \
        }                                                         \
    } while (false)
// NOLINTEND(bugprone-macro-parentheses)


////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_PRIV_OPTIONAL_DESTROY(T, buffer)                  \
    do                                                         \
    {                                                          \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T)) \
        {                                                      \
            buffer.obj.~T();                                   \
        }                                                      \
    } while (false)

// NOLINTEND(bugprone-macro-parentheses)


////////////////////////////////////////////////////////////
/// \brief Lightweight replacement for `std::optional`
///
/// Wraps an instance of `T` together with a boolean engagement flag,
/// avoiding the heavy `<optional>` standard header. Storage is in-place
/// and never allocates. Special member functions are conditionally
/// `default`ed when `T` is trivially copy/move/destructible to keep the
/// type trivial whenever possible.
///
/// `Optional<T>` propagates trivial relocatability from `T` so that it
/// can be moved with `memcpy` inside SFML containers when applicable.
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Optional
{
public:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        enableTrivialRelocation = SFML_BASE_IS_TRIVIALLY_RELOCATABLE(T)
    };


    ////////////////////////////////////////////////////////////
    /// \brief Default constructor, creates an empty optional
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional() noexcept : m_engaged{false}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an empty optional from `nullOpt`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional(NullOpt) noexcept : m_engaged{false}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an engaged optional by copying `object`
    ///
    /// \param object Value to copy into the optional
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(const T& object) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(object);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an engaged optional by moving `object`
    ///
    /// \param object Value to move into the optional
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(T&& object) noexcept : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(object));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor (non-trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional(const Optional& rhs)
        requires(!isTriviallyCopyConstructible<T> && isCopyConstructible<T>)
        : m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(rhs.m_buffer.obj);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor (trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional(const Optional& rhs)
        requires(isTriviallyCopyConstructible<T>)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Move constructor (non-trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional(Optional&& rhs) noexcept
        requires(!isTriviallyMoveConstructible<T> && isMoveConstructible<T>)
        : m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(rhs.m_buffer.obj));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move constructor (trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ Optional(Optional&& rhs)
        requires(isTriviallyMoveConstructible<T>)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Destructor, destroys the contained value if engaged (non-trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr ~Optional() noexcept
        requires(!isTriviallyDestructible<T>)
    {
        if (m_engaged)
            m_buffer.obj.~T();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destructor (trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr ~Optional() noexcept
        requires(isTriviallyDestructible<T>)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment (non-trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(const Optional& rhs)
        requires(!isTriviallyCopyAssignable<T> && isCopyAssignable<T>)
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_OPTIONAL_DESTROY(T, m_buffer);
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(rhs.m_buffer.obj);
        }
        else
        {
            SFML_BASE_ASSERT(m_engaged && rhs.m_engaged);
            m_buffer.obj = rhs.m_buffer.obj;
        }

        return *this;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment (trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(const Optional& rhs)
        requires(isTriviallyCopyAssignable<T>)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Move assignment (non-trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(Optional&& rhs) noexcept
        requires(!isTriviallyMoveAssignable<T> && isMoveAssignable<T>)
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_OPTIONAL_DESTROY(T, m_buffer);
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(rhs.m_buffer.obj));
        }
        else
        {
            SFML_BASE_ASSERT(m_engaged && rhs.m_engaged);
            m_buffer.obj = SFML_BASE_MOVE(rhs.m_buffer.obj);
        }

        return *this;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move assignment (trivial overload)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(Optional&& rhs)
        requires(isTriviallyMoveAssignable<T>)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Disabled in-place constructor: copying another optional makes no sense here
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, const Optional&) = delete;


    ////////////////////////////////////////////////////////////
    /// \brief Disabled in-place constructor: moving another optional makes no sense here
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Optional&&) = delete;


    ////////////////////////////////////////////////////////////
    /// \brief Construct an engaged optional in-place from constructor arguments
    ///
    /// Forwards `args...` to `T`'s constructor without any intermediate copy or move.
    ///
    /// \param args Arguments forwarded to `T`'s constructor
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Args&&... args) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an engaged optional from the result of an invocable
    ///
    /// Equivalent to constructing the value from `func()` while still benefiting
    /// from guaranteed copy elision.
    ///
    /// \param func Invocable returning a value convertible to `T`
    ///
    ////////////////////////////////////////////////////////////
    template <typename F>
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(FromFunc, F&& func) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(func)());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Replace the contained value, constructing a new one in-place
    ///
    /// Destroys the previous value if any, then constructs a new `T` from `args...`.
    ///
    /// \param args Arguments forwarded to `T`'s constructor
    ///
    /// \return Reference to the newly constructed value
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    [[gnu::always_inline]] constexpr T& emplace(Args&&... args)
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = true;

        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(args)...));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Replace the contained value with the result of an invocable
    ///
    /// Destroys the previous value if any, then constructs a new `T`
    /// from `func()`. Useful when `T` is not movable.
    ///
    /// \param func Invocable returning a value convertible to `T`
    ///
    /// \return Reference to the newly constructed value
    ///
    ////////////////////////////////////////////////////////////
    template <typename F>
    [[gnu::always_inline]] constexpr T& emplaceFromFunc(F&& func)
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = true;

        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(func)()));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a value in-place only if currently empty
    ///
    /// If the optional is already engaged, this is a no-op and the existing
    /// value is returned. Otherwise behaves like `emplace`.
    ///
    /// \param args Arguments forwarded to `T`'s constructor when needed
    ///
    /// \return Reference to the contained value
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    [[gnu::always_inline]] constexpr T& emplaceIfNeeded(Args&&... args)
    {
        if (m_engaged) [[likely]]
            return m_buffer.obj;

        m_engaged = true;
        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(args)...));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a value from an invocable only if currently empty
    ///
    /// If the optional is already engaged, this is a no-op. Otherwise
    /// behaves like `emplaceFromFunc`.
    ///
    /// \param func Invocable returning a value convertible to `T` when needed
    ///
    /// \return Reference to the contained value
    ///
    ////////////////////////////////////////////////////////////
    template <typename F>
    [[gnu::always_inline]] constexpr T& emplaceFromFuncIfNeeded(F&& func)
    {
        if (m_engaged) [[likely]]
            return m_buffer.obj;

        m_engaged = true;
        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(func)()));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Disengage the optional, destroying the contained value if any
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void reset() noexcept
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = false;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Access the contained value, throwing if empty
    ///
    /// \throw `BadOptionalAccess` (or aborts) when the optional is empty
    ///
    /// \return Reference to the contained value
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& value() &
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Access the contained value, throwing if empty (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const T& value() const&
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Access the contained value, throwing if empty (rvalue overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T&& value() &&
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return SFML_BASE_MOVE(m_buffer.obj);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the contained value if engaged, otherwise `defaultValue`
    ///
    /// Unlike `std::optional::value_or`, this overload returns by reference
    /// to avoid an unnecessary copy of `defaultValue` in the empty case.
    ///
    /// \param defaultValue Value to return if the optional is empty
    ///
    /// \return Reference to either the contained value or `defaultValue`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& valueOr(T& defaultValue) & noexcept
    {
        return m_engaged ? m_buffer.obj : defaultValue;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the contained value if engaged, otherwise `defaultValue` (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& valueOr(const T& defaultValue) const& noexcept
    {
        return m_engaged ? m_buffer.obj : defaultValue;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the contained value if engaged, otherwise `defaultValue` (rvalue overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& valueOr(T&& defaultValue) && noexcept
    {
        return SFML_BASE_MOVE(m_engaged ? m_buffer.obj : defaultValue);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether the optional contains a value
    ///
    /// \return `true` if engaged, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool hasValue() const noexcept
    {
        return m_engaged;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Conversion to `bool`, equivalent to `hasValue()`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit operator bool() const noexcept
    {
        return m_engaged;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Member access on the contained value (asserts engagement)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* operator->() & noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return &m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Member access on the contained value (const overload, asserts engagement)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* operator->() const& noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return &m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dereference the contained value (asserts engagement)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator*() & noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dereference the contained value (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& operator*() const& noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return m_buffer.obj;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dereference the contained value (rvalue overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& operator*() && noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return SFML_BASE_MOVE(m_buffer.obj);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the contained value, or `nullptr` if empty
    ///
    /// Unlike `operator->`, this is safe to call on an empty optional.
    ///
    /// \return Pointer to the value, or `nullptr`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* asPtr() noexcept
    {
        return m_engaged ? &m_buffer.obj : nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the contained value, or `nullptr` if empty (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* asPtr() const noexcept
    {
        return m_engaged ? &m_buffer.obj : nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator==(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
        requires requires { *lhs == *rhs; }
    {
        return lhs.m_engaged == rhs.m_engaged && (!lhs.m_engaged || *lhs == *rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator!=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
        requires requires { *lhs != *rhs; }
    {
        return lhs.m_engaged != rhs.m_engaged || (lhs.m_engaged && *lhs != *rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
        requires requires { *lhs >= *rhs; }
    {
        return !rhs.m_engaged || (lhs.m_engaged && *lhs >= *rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
        requires requires { *lhs <= *rhs; }
    {
        return !lhs.m_engaged || (rhs.m_engaged && *lhs <= *rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>(const Optional& lhs,
                                                                                        const Optional& rhs) noexcept
        requires requires { *lhs > *rhs; }
    {
        return lhs.m_engaged && (!rhs.m_engaged || *lhs > *rhs);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<(const Optional& lhs,
                                                                                        const Optional& rhs) noexcept
        requires requires { *lhs < *rhs; }
    {
        return rhs.m_engaged && (!lhs.m_engaged || *lhs < *rhs);
    }

private:
    union Buffer
    {
        char dummy{}; // Needed by GCC for constant expression support
        T    obj;

        // clang-format off
        constexpr Buffer() requires(isTriviallyConstructible<T>) = default;
        constexpr Buffer() requires(!isTriviallyConstructible<T>) { }

        constexpr ~Buffer() requires(isTriviallyDestructible<T>) = default;
        constexpr ~Buffer() requires(!isTriviallyDestructible<T>) { }

        constexpr Buffer(const Buffer&) = default;
        constexpr Buffer& operator=(const Buffer&) = default;

        constexpr Buffer(Buffer&&) = default;
        constexpr Buffer& operator=(Buffer&&) = default;
        // clang-format on
    } m_buffer;

    bool m_engaged;
};

////////////////////////////////////////////////////////////
#undef SFML_PRIV_OPTIONAL_DESTROY
#undef SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED


////////////////////////////////////////////////////////////
/// \brief Construct an `Optional` whose value type is deduced from `object`
///
/// The element type is `RemoveCVRef<Object>`. Forwards `object` into the
/// new optional.
///
////////////////////////////////////////////////////////////
template <typename Object>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto makeOptional(Object&& object)
{
    return Optional<SFML_BASE_REMOVE_CVREF(Object)>{SFML_BASE_FORWARD(object)};
}


////////////////////////////////////////////////////////////
/// \brief Construct an `Optional<T>` in-place from constructor arguments
///
/// Forwards `args...` to `T`'s constructor without intermediate copies.
///
////////////////////////////////////////////////////////////
template <typename T, typename... Args>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Optional<T> makeOptional(Args&&... args)
{
    return Optional<T>{inPlace, SFML_BASE_FORWARD(args)...};
}


////////////////////////////////////////////////////////////
/// \brief Construct an `Optional` from the result of an invocable
///
/// The element type is deduced from the return type of `f()`. The result
/// is constructed in place inside the optional with guaranteed copy
/// elision, so this also works with non-movable types.
///
////////////////////////////////////////////////////////////
template <typename F>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto makeOptionalFromFunc(F&& f)
{
    return Optional<decltype(SFML_BASE_FORWARD(f)())>{fromFunc, SFML_BASE_FORWARD(f)};
}

} // namespace sf::base
