/*
 * SDL3 Bug: SDL_AbortAssertion → SDL_Quit infinite recursion on Emscripten
 * ========================================================================
 *
 * SUMMARY
 * -------
 * When an SDL assertion fires and the handler returns SDL_ASSERTION_ABORT,
 * SDL_AbortAssertion() calls SDL_Quit() before SDL_abort(). If SDL_Quit's
 * cleanup path triggers another assertion, the nested assertion handler
 * calls SDL_AbortAssertion → SDL_Quit again, creating mutual recursion.
 *
 * The `assertion_running` counter in SDL_ReportAssertion guards against
 * this: at depth 2 it calls SDL_AbortAssertion (which recurses one more
 * level), and at depth 3 it calls SDL_abort(). On desktop this terminates
 * the process. On Emscripten with Asyncify, however, abort() throws a JS
 * exception that Asyncify's try/catch can swallow, so execution continues
 * and the call stack grows until the browser throws:
 *   "RangeError: Maximum call stack size exceeded"
 *
 * AFFECTED CODE (SDL_assert.c)
 * ----------------------------
 *   static SDL_NORETURN void SDL_AbortAssertion(void)
 *   {
 *       SDL_Quit();    // ← can re-trigger assertions during cleanup
 *       SDL_abort();   // ← on Emscripten+Asyncify, may not halt
 *   }
 *
 * REAL-WORLD CRASH (VRSFML on Emscripten)
 * ----------------------------------------
 * VRSFML initializes SDL3 with SDL_INIT_VIDEO and lazily SDL_INIT_JOYSTICK.
 * During program shutdown on Emscripten, an assertion fires in SDL's
 * joystick/gamepad quit path. The resulting recursion produces:
 *
 *   SDL_ReportAssertion (SDL_assert.c)
 *   SDL_AssertJoysticksLocked (SDL_joystick.c)
 *   SDL_PrivateGamepadRemoved (SDL_gamepad.c)
 *   SDL_QuitGamepads (SDL_gamepad.c)
 *   SDL_QuitSubSystem (SDL.c)
 *   SDL_Quit (SDL.c)
 *   SDL_AbortAssertion (SDL_assert.c)
 *   SDL_ReportAssertion (SDL_assert.c)       ← repeats until stack overflow
 *   ...
 *
 * REPRODUCER
 * ----------
 * This program forces the ABORT path by installing a custom assertion
 * handler, then triggers SDL_assert(false). On Emscripten with Asyncify,
 * the resulting SDL_AbortAssertion → SDL_Quit → cleanup assertions →
 * SDL_AbortAssertion chain overflows the stack.
 *
 * Build for Emscripten (link against SDL3 built from source):
 *   emcc sdl3_emscripten_assert_recursion_repro.c \
 *     -I<SDL3>/include/SDL3 -L<SDL3>/build -lSDL3 \
 *     -sASYNCIFY -sASSERTIONS=1 -g -o repro.html
 *
 * SUGGESTED FIXES
 * ---------------
 * 1. Remove SDL_Quit() from SDL_AbortAssertion — the process is about to
 *    abort, so cleanup is unnecessary and dangerous.
 *
 * 2. Guard SDL_Quit() in SDL_AbortAssertion with a static re-entry flag:
 *      static SDL_NORETURN void SDL_AbortAssertion(void)
 *      {
 *          static bool aborting = false;
 *          if (!aborting) { aborting = true; SDL_Quit(); }
 *          SDL_abort();
 *      }
 *
 * 3. On Emscripten, use __builtin_trap() or emscripten_force_exit()
 *    instead of abort() in SDL_abort(), so execution actually halts
 *    even under Asyncify.
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

/* Force ABORT on every assertion — this matches the Emscripten default
 * behavior when SDL_ShowMessageBox fails and the built-in prompt
 * handler cannot block (e.g. inside Asyncify rewind). */
static SDL_AssertState SDLCALL always_abort(
    const SDL_AssertData *data, void *userdata)
{
    (void)userdata;
    fprintf(stderr, "[repro] Assertion ABORT: %s (%s:%d)\n",
            data->condition, data->filename, data->linenum);
    return SDL_ASSERTION_ABORT;
}

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetAssertionHandler(always_abort, NULL);

    /*
     * Trigger an assertion.  The handler returns ABORT, so:
     *   SDL_ReportAssertion  (assertion_running = 1)
     *     → always_abort() → SDL_ASSERTION_ABORT
     *     → SDL_AbortAssertion()
     *       → SDL_Quit()          ← cleanup runs; if any internal
     *         → ...                  assertion fires here, recursion
     *       → SDL_abort()         ← on Emscripten+Asyncify this may
     *                                not halt, and the stack overflows
     *
     * On desktop the process terminates at SDL_abort().
     * On Emscripten with Asyncify it may stack-overflow instead.
     */
    fprintf(stderr, "[repro] Triggering SDL_assert(false)...\n");
    SDL_assert(false);

    /* Not reached on desktop; may be reached on Emscripten if abort()
     * didn't halt. */
    fprintf(stderr, "[repro] ERROR: execution continued past SDL_abort!\n");
    return 1;
}
