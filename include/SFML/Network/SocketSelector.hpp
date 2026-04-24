#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Export.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Span.hpp"


namespace sf
{
class Socket;

////////////////////////////////////////////////////////////
/// \brief Multiplexer that waits for readability and/or writability on multiple sockets
///
////////////////////////////////////////////////////////////
class SFML_NETWORK_API SocketSelector
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief One entry in the ready-set views returned by
    ///        `getReadyToReceive` / `getReadyToSend`
    ///
    /// `socket` is the socket that became ready; `userData` is
    /// whatever opaque pointer the caller attached at `add` /
    /// `addForSend` time (or `nullptr` if none was attached).
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] ReadyEntry
    {
        Socket* socket   = nullptr;
        void*   userData = nullptr;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SocketSelector();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SocketSelector();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector(const SocketSelector&);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector& operator=(const SocketSelector&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector(SocketSelector&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    SocketSelector& operator=(SocketSelector&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Watch `socket` for readability (or a pending listener accept)
    ///
    /// Idempotent: if `socket` is already being watched for
    /// read, this does nothing. If it was previously registered
    /// only for write (via `addForSend`), this extends the
    /// registration -- the socket will now fire on either.
    ///
    /// The selector keeps a non-owning mutable reference to the
    /// socket (it is handed back to the caller via the ready
    /// lists, so the caller can call `send`/`receive` on it);
    /// the caller must ensure the socket outlives the selector
    /// (or is removed before it dies).
    ///
    /// `userData` is an opaque pointer the caller can attach to
    /// the registration. It is reported back alongside the
    /// socket in `getReadyToReceive()`'s results so the caller
    /// can recover whatever owner / context object it belongs
    /// to without maintaining a side-table. Each call to `add`
    /// or `addForSend` on the same socket replaces the stored
    /// `userData` with whatever is passed in (default
    /// `nullptr`).
    ///
    /// \param socket   Reference to the socket to add
    /// \param userData Opaque pointer reported back in `ReadyEntry::userData` (default: `nullptr`)
    ///
    /// \return `false` if an error occurs, `true` otherwise
    ///
    /// \see `addForSend`, `remove`, `clear`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool add(Socket& socket, void* userData = nullptr);

    ////////////////////////////////////////////////////////////
    /// \brief Watch `socket` for writability
    ///
    /// Idempotent: if `socket` is already being watched for
    /// write, this does nothing. If it was previously
    /// registered only for read (via `add`), this extends the
    /// registration -- the socket will now fire on either.
    ///
    /// The selector keeps a non-owning mutable reference to the
    /// socket (it is handed back to the caller via the ready
    /// lists, so the caller can call `send`/`receive` on it);
    /// the caller must ensure the socket outlives the selector
    /// (or is removed before it dies).
    ///
    /// Each call to `add` or `addForSend` on the same socket
    /// replaces the stored `userData` with whatever is passed
    /// in (default `nullptr`) -- see `add` for details.
    ///
    /// \param socket   Reference to the socket to add
    /// \param userData Opaque pointer reported back in `ReadyEntry::userData` (default: `nullptr`)
    ///
    /// \return `false` if an error occurs, `true` otherwise
    ///
    /// \see `add`, `remove`, `clear`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool addForSend(Socket& socket, void* userData = nullptr);

    ////////////////////////////////////////////////////////////
    /// \brief Remove a socket from the selector
    ///
    /// Removes any read and/or write registration for the given
    /// socket. After removal the socket is no longer observed
    /// and will not appear in the ready lists or be reported by
    /// `isReady` / `isReadyToSend`.
    ///
    /// \param socket Reference to the socket to remove
    ///
    /// \return `false` if an error occurs, `true` otherwise
    ///
    /// \see `add`, `addForSend`, `clear`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool remove(const Socket& socket);

    ////////////////////////////////////////////////////////////
    /// \brief Remove all the sockets stored in the selector
    ///
    /// The selector is left empty; observed sockets themselves
    /// are unaffected.
    ///
    /// \see `add`, `addForSend`, `remove`
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    ////////////////////////////////////////////////////////////
    /// \brief Wait until one or more observed sockets become ready
    ///
    /// Returns when any socket registered via `add` becomes
    /// readable, any socket registered via `addForSend` becomes
    /// writable, or the timeout expires. After a successful
    /// return, the ready set is available via `getReadyToReceive`
    /// / `getReadyToSend` and point-checkable via `isReady` /
    /// `isReadyToSend`. A subsequent call to `wait` rebuilds
    /// the ready set.
    ///
    /// \param timeout Maximum time to wait; `Time{}` waits forever
    ///
    /// \return `true` if at least one socket is ready, `false` on timeout
    ///
    /// \see `getReadyToReceive`, `getReadyToSend`, `isReady`, `isReadyToSend`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool wait(Time timeout = {});

    ////////////////////////////////////////////////////////////
    /// \brief Sockets that became ready to receive on the last `wait`
    ///
    /// Contains only sockets that were registered via `add` and
    /// that the last `wait` reported as readable. Each
    /// `ReadyEntry` bundles the ready `Socket*` with the
    /// `userData` pointer the caller attached at registration
    /// time.
    ///
    /// The returned view is valid until the next call to `wait`
    /// / `add` / `addForSend` / `remove` / `clear`. Iterating
    /// is O(M) where M is the ready count.
    ///
    /// \see `wait`, `getReadyToSend`, `ReadyEntry`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Span<const ReadyEntry> getReadyToReceive() const;

    ////////////////////////////////////////////////////////////
    /// \brief Sockets that became ready to send on the last `wait`
    ///
    /// Contains only sockets that were registered via
    /// `addForSend` and that the last `wait` reported as
    /// writable. Each `ReadyEntry` bundles the ready `Socket*`
    /// with the `userData` pointer the caller attached at
    /// registration time.
    ///
    /// The returned view is valid until the next call to `wait`
    /// / `add` / `addForSend` / `remove` / `clear`. Iterating
    /// is O(M) where M is the ready count.
    ///
    /// \see `wait`, `getReadyToReceive`, `ReadyEntry`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Span<const ReadyEntry> getReadyToSend() const;

    ////////////////////////////////////////////////////////////
    /// \brief Test whether a specific socket is ready to receive
    ///
    /// Reflects the state of the most recent `wait` call.
    /// Before any `wait` has run, and after a `wait` that
    /// timed out, this returns `false` for every socket. A
    /// socket that was not registered via `add` can never
    /// return `true`.
    ///
    /// \param socket Socket to test
    ///
    /// \return `true` if `socket` was reported ready to receive by the last `wait`, `false` otherwise
    ///
    /// \see `wait`, `getReadyToReceive`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isReady(const Socket& socket) const;

    ////////////////////////////////////////////////////////////
    /// \brief Test whether a specific socket is ready to send
    ///
    /// Reflects the state of the most recent `wait` call.
    /// Before any `wait` has run, and after a `wait` that
    /// timed out, this returns `false` for every socket. A
    /// socket that was not registered via `addForSend` can
    /// never return `true`.
    ///
    /// \param socket Socket to test
    ///
    /// \return `true` if `socket` was reported ready to send by the last `wait`, `false` otherwise
    ///
    /// \see `wait`, `getReadyToSend`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isReadyToSend(const Socket& socket) const;

private:
    ////////////////////////////////////////////////////////////
    // Forward-declared pImpl type (must precede `addWith`, which takes it by reference)
    ////////////////////////////////////////////////////////////
    struct Impl;

    ////////////////////////////////////////////////////////////
    /// \brief Shared body for `add` / `addForSend`
    ///
    /// Implemented as a private static member so it can see
    /// both `Impl` (declared inside this class) and
    /// `Socket::getNativeHandle()` (friendship on `Socket`).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool addWith(Impl& impl, Socket& socket, void* userData, bool forReceive, bool forSend);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::InPlacePImpl<Impl, 4096> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SocketSelector
/// \ingroup network
///
/// Socket selectors let a single thread wait for activity on
/// many sockets at once. Each registered socket is watched for
/// readability (`add`), writability (`addForSend`), or both
/// (by calling both). `wait` blocks until at least one watched
/// event fires (or the timeout expires).
///
/// All socket types can be registered:
/// \li `sf::TcpListener`
/// \li `sf::TcpSocket`
/// \li `sf::UdpSocket`
///
/// A selector holds non-owning references -- it does not copy
/// or store the sockets themselves. Callers must ensure
/// registered sockets outlive the selector (or are removed
/// before they die).
///
/// Typical usage:
/// \li register every socket you want to observe via `add` and/or `addForSend`
/// \li loop: `wait()`, then either iterate `getReadyToReceive` / `getReadyToSend`
///   or point-check specific sockets with `isReady` / `isReadyToSend`
///
/// Usage example (accept + echo):
/// \code
/// auto listener = sf::TcpListener::create(55001, /* blocking */ true).value();
/// sf::base::Vector<sf::TcpSocket> clients;
///
/// sf::SocketSelector selector;
/// (void)selector.add(listener);
///
/// while (running)
/// {
///     if (!selector.wait(sf::milliseconds(100)))
///         continue;
///
///     // New connection?
///     if (selector.isReady(listener))
///     {
///         auto result = listener.accept();
///         if (result.status == sf::Socket::Status::Done)
///         {
///             auto& client = clients.emplaceBack(SFML_BASE_MOVE(result.socket));
///             (void)selector.add(client);
///         }
///     }
///
///     // Iterate only the ready clients (O(M), not O(N))
///     for (const sf::Socket* sock : selector.getReadyToReceive())
///     {
///         if (sock == &listener)
///             continue;
///
///         // ... receive / dispatch ...
///     }
/// }
/// \endcode
///
/// To wait for writability (e.g. to send only when the kernel
/// send buffer has room) register the socket via `addForSend`
/// and inspect `getReadyToSend()` / `isReadyToSend` after each
/// `wait`.
///
/// \see `sf::Socket`
///
////////////////////////////////////////////////////////////
