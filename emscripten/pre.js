// Lift V8/SpiderMonkey's `Error.stackTraceLimit` (default 10) so that SFML's
// `printStackTrace` -- which reads `console.trace()` and
// `emscripten_get_callstack(EM_LOG_JS_STACK)` -- can show the full chain back
// to the failing test/caller, not just the deepest few frames.
if (typeof Error !== 'undefined') {
    Error.stackTraceLimit = Infinity;
}
