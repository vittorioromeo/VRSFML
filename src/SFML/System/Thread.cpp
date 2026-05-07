// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers (project)
////////////////////////////////////////////////////////////
#include "SFML/System/Thread.hpp"

#include "SFML/Config.hpp"

#include "SFML/System/Atomic.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Memset.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Launder.hpp"


////////////////////////////////////////////////////////////
// Platform backend selection
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_ANDROID) || defined(SFML_SYSTEM_MACOS) || \
    defined(SFML_SYSTEM_IOS) || defined(SFML_SYSTEM_FREEBSD) || defined(SFML_SYSTEM_OPENBSD) || \
    defined(SFML_SYSTEM_NETBSD) || defined(SFML_SYSTEM_EMSCRIPTEN)
    #define SFML_THREAD_POSIX 1
#elif defined(SFML_SYSTEM_WINDOWS)
    #define SFML_THREAD_WIN32 1
#else
    #error "sf::Thread: no thread backend implemented for this platform"
#endif


////////////////////////////////////////////////////////////
// Headers (platform)
////////////////////////////////////////////////////////////
#if SFML_THREAD_POSIX

    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>

    #include <cerrno>
    #include <ctime>

    #if defined(SFML_SYSTEM_MACOS) || defined(SFML_SYSTEM_IOS)
        #include <sys/sysctl.h>
    #endif

#elif SFML_THREAD_WIN32

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include "SFML/System/WindowsHeader.hpp"

    #include <mmsystem.h>
    #include <process.h>
    #include <synchapi.h>

#endif


namespace sf
{
////////////////////////////////////////////////////////////
// Per-thread id slot. Declared at namespace scope (not in the
// anonymous namespace) so the trampoline can refer to a single
// shared TLS symbol across this TU.
////////////////////////////////////////////////////////////
thread_local base::U64 tlCurrentThreadId{0u};


namespace
{
////////////////////////////////////////////////////////////
// Reinterpret the opaque `m_native[16]` byte buffer as the
// platform handle. The `static_assert`s below catch any ABI
// surprise (pthread_t larger than expected, etc.) at compile time.
////////////////////////////////////////////////////////////
#if SFML_THREAD_POSIX

using NativeHandle = pthread_t;

static_assert(sizeof(NativeHandle) <= 16u);
static_assert(alignof(NativeHandle) <= alignof(base::U64));

#elif SFML_THREAD_WIN32

struct NativeHandle
{
    HANDLE handle;
    DWORD  id;
};

static_assert(sizeof(NativeHandle) <= 16u);
static_assert(alignof(NativeHandle) <= alignof(base::U64));

#endif


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline NativeHandle& asNative(void* const p) noexcept
{
    // `m_native` is a raw byte buffer; `pthread_create` / `_beginthreadex`
    // construct the `NativeHandle` in place. `SFML_BASE_LAUNDER_CAST` tells
    // the compiler to treat the bytes as a fresh `NativeHandle*` (i.e. drop
    // any value-tracking that assumed it was still `unsigned char[16]`).
    return *SFML_BASE_LAUNDER_CAST(NativeHandle*, p);
}


////////////////////////////////////////////////////////////
// Monotonic counter for `ThreadId`. The first time a given
// thread asks for its id, it grabs the next value here.
////////////////////////////////////////////////////////////
constinit Atomic<base::U64> gNextThreadId{1u};


////////////////////////////////////////////////////////////
// Run the user callable + free the single-block entry. The
// header instantiates `entry->deinit` per `F` and embeds the
// payload's offset+alignment into the thunk, so this function
// stays type-erased.
//
// `noexcept` so the underlying OS doesn't see a stack unwind
// cross the trampoline frame; the user routine is itself
// `noexcept`, so an escaping exception in user code triggers
// `std::terminate` before reaching the platform thunk.
////////////////////////////////////////////////////////////
inline void runEntry(priv::ThreadEntry* const entry) noexcept
{
    // Establish this thread's id BEFORE running user code, so
    // the user's first `ThisThread::getId()` returns the same
    // value as `Thread::getId()` on the spawner side.
    tlCurrentThreadId = entry->id;

    // Runs the callable, destroys it, and frees the entry block.
    entry->deinit(entry, /* runFirst */ true);
}


////////////////////////////////////////////////////////////
// Per-platform start-routine signature wraps the noexcept body.
////////////////////////////////////////////////////////////
#if SFML_THREAD_POSIX

extern "C" void* threadStartRoutine(void* arg)
{
    runEntry(static_cast<priv::ThreadEntry*>(arg));
    return nullptr;
}

#elif SFML_THREAD_WIN32

extern "C" unsigned __stdcall threadStartRoutine(void* arg)
{
    runEntry(static_cast<priv::ThreadEntry*>(arg));
    return 0u;
}

#endif

} // namespace


////////////////////////////////////////////////////////////
Thread::Thread(int, priv::ThreadEntry* entry)
{
    // `entry` already owns the user callable (built in the header
    // factory). We just stamp its id and hand it to the kernel.
    entry->id = gNextThreadId.fetchAddRelaxed(1u);
    m_id      = entry->id;

#if SFML_THREAD_POSIX

    if (const int rc = pthread_create(&asNative(m_native), /* attr */ nullptr, &threadStartRoutine, entry); rc != 0)
    {
        // Destroys the user callable and frees the single-block entry
        // (without running the callable, since spawn never happened).
        entry->deinit(entry, /* runFirst */ false);
        base::abort();
    }

#elif SFML_THREAD_WIN32

    auto& native  = asNative(m_native);
    native.handle = reinterpret_cast<HANDLE>(
        _beginthreadex(/* security    */ nullptr,
                       /* stack_size  */ 0u,
                       &threadStartRoutine,
                       entry,
                       /* initflag    */ 0u,
                       reinterpret_cast<unsigned int*>(&native.id)));

    if (native.handle == nullptr)
    {
        // Same single-block cleanup as the POSIX failure path above.
        entry->deinit(entry, /* runFirst */ false);
        base::abort();
    }

#endif

    m_joinable = true;
}


////////////////////////////////////////////////////////////
void Thread::takeFrom(Thread& dst, Thread& other) noexcept
{
    SFML_BASE_MEMCPY(dst.m_native, other.m_native, sizeof(dst.m_native));
    dst.m_id       = other.m_id;
    dst.m_joinable = other.m_joinable;

    SFML_BASE_MEMSET(other.m_native, 0, sizeof(other.m_native));
    other.m_id       = 0u;
    other.m_joinable = false;
}


////////////////////////////////////////////////////////////
Thread::Thread(Thread&& other) noexcept
{
    takeFrom(*this, other);
}


////////////////////////////////////////////////////////////
Thread& Thread::operator=(Thread&& other) noexcept
{
    if (this == &other)
        return *this;

    // `std::jthread`-style semantics: an existing joinable thread is
    // implicitly joined before the slot is overwritten.
    if (m_joinable)
        join();

    takeFrom(*this, other);
    return *this;
}


////////////////////////////////////////////////////////////
Thread::~Thread()
{
    if (m_joinable)
        join();
}


////////////////////////////////////////////////////////////
void Thread::join()
{
    SFML_BASE_ASSERT(m_joinable);

#if SFML_THREAD_POSIX

    pthread_join(asNative(m_native), /* retval */ nullptr);

#elif SFML_THREAD_WIN32

    auto& native = asNative(m_native);
    WaitForSingleObject(native.handle, INFINITE);
    CloseHandle(native.handle);
    native.handle = nullptr;

#endif

    m_joinable = false;
}


////////////////////////////////////////////////////////////
void Thread::detach()
{
    SFML_BASE_ASSERT(m_joinable);

#if SFML_THREAD_POSIX

    pthread_detach(asNative(m_native));

#elif SFML_THREAD_WIN32

    auto& native = asNative(m_native);
    CloseHandle(native.handle);
    native.handle = nullptr;

#endif

    m_joinable = false;
}


////////////////////////////////////////////////////////////
bool Thread::joinable() const noexcept
{
    return m_joinable;
}


////////////////////////////////////////////////////////////
ThreadId Thread::getId() const noexcept
{
    return ThreadId{m_id};
}


////////////////////////////////////////////////////////////
unsigned int Thread::hardwareConcurrency() noexcept
{
#if SFML_THREAD_POSIX

    const long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? static_cast<unsigned int>(n) : 0u;

#elif SFML_THREAD_WIN32

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<unsigned int>(info.dwNumberOfProcessors);

#endif
}


////////////////////////////////////////////////////////////
ThreadId ThisThread::getId() noexcept
{
    if (tlCurrentThreadId == 0u) [[unlikely]]
        tlCurrentThreadId = gNextThreadId.fetchAddRelaxed(1u);

    return ThreadId{tlCurrentThreadId};
}


////////////////////////////////////////////////////////////
void ThisThread::yield() noexcept
{
#if SFML_THREAD_POSIX
    sched_yield();
#elif SFML_THREAD_WIN32
    SwitchToThread();
#endif
}


////////////////////////////////////////////////////////////
void ThisThread::sleepFor(Time duration)
{
    if (duration <= Time{})
        return;

#if SFML_THREAD_POSIX

    const base::I64 usecs = duration.asMicroseconds();

    timespec ts{};
    ts.tv_sec  = static_cast<time_t>(usecs / 1'000'000);
    ts.tv_nsec = static_cast<long>((usecs % 1'000'000) * 1000);

    // Restart on EINTR using the remaining time the kernel writes back.
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
    {
    }

#elif SFML_THREAD_WIN32

    // Bump the system timer resolution so `Sleep` honors short
    // durations more accurately. `std::this_thread::sleep_for` is
    // intentionally not used: it produces inconsistent results
    // under MinGW-w64.
    static const UINT periodMin = []
    {
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        return tc.wPeriodMin;
    }();

    timeBeginPeriod(periodMin);
    ::Sleep(static_cast<DWORD>(duration.asMilliseconds()));
    timeEndPeriod(periodMin);

#endif
}

} // namespace sf
