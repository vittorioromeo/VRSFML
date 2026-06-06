# detect the OS
if(WIN32)
    set(ZA_OS_WINDOWS 1)

    # detect the architecture
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64" OR "${MSVC_CXX_ARCHITECTURE_ID}" MATCHES "ARM64" OR "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "ARM64")
        set(ZA_ARCH_ARM64 1)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ZA_ARCH_X86 1)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ZA_ARCH_X64 1)
    else()
        message(FATAL_ERROR "Unsupported architecture")
    endif()
elseif(ANDROID)
    set(ZA_OS_ANDROID 1)
    set(ZA_OS_UNIX 1)
elseif(IOS)
    set(ZA_OS_IOS 1)
elseif(APPLE)
    set(ZA_OS_MACOS 1)
elseif(EMSCRIPTEN)
    message(STATUS "Detected Emscripten")
    set(ZA_OS_EMSCRIPTEN 1)

    set(ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_DEBUG
        -g3          # Enable debug mode
        -gsource-map # Generate a source map using LLVM debug information
        -O0          # Disable optimizations
    )

    set(ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELEASE
        -O3          # Enable optimizations
        -flto        # Link-time optimization
        -DNDEBUG=1   # Should be set by CMake, but I'm paranoid
    )

    set(ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELWITHDEBINFO
        -g3          # Enable debug mode
        -gsource-map # Generate a source map using LLVM debug information
        -O3          # Enable optimizations
        -flto        # Link-time optimization
        -DNDEBUG=1   # Should be set by CMake, but I'm paranoid
    )

    set(ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS
        $<$<CONFIG:Debug>:${ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_DEBUG}>
        $<$<CONFIG:Release>:${ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELEASE}>
        $<$<CONFIG:RelWithDebInfo>:${ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELWITHDEBINFO}>

        -pthread     # Enable threading support

        --shell-file=${CMAKE_SOURCE_DIR}/emscripten/shell.html
    )

    # -sEXCEPTION_STACK_TRACES=1 # Exceptions will contain stack traces and uncaught exceptions will display stack traces
    # -sDETERMINISTIC=1          # Force `Date.now()`, `Math.random`, etc. to return deterministic results
    # -fwasm-exceptions          # TODO P1: -fwasm-exceptions seems to break examples
    # -sEXCEPTION_DEBUG=1        # Print out exceptions in emscriptened code (SPEWS WARNINGS)
    # -sGL_DEBUG=1               # Enables more verbose debug printing of WebGL related operations (SPEWS WARNINGS)

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_DEBUG
        -g3                                 # Enable debug mode
        -gsource-map                        # Generate a source map using LLVM debug information
        --profiling-funcs                   # Preserves names in the WASM binary

        -sASSERTIONS=2                      # Add runtime assertions
        -sCHECK_NULL_WRITES=1               # Help detect `NULL` pointer usage
        -sSAFE_HEAP=1                       # Check each write to the heap
        -sSTACK_OVERFLOW_CHECK=1            # Adds a security cookie at the top of the stack
        -sGL_ASSERTIONS=1                   # Adds extra checks for error situations in the GL library
    )

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELEASE
        -O3                                 # Enable advanced linker optimizations
        -flto                               # Link-time optimization

        -SMINIFY_HTML=1                     # Runs generated `.html` file through `html-minifier`
    )

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELWITHDEBINFO
        -O3                                 # Enable advanced linker optimizations
        -flto                               # Link-time optimization
        -g3                                 # Enable debug mode
        -gsource-map                        # Generate a source map using LLVM debug information

        -SMINIFY_HTML=1                     # Runs generated `.html` file through `html-minifier`
    )

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_ASYNCIFY
        -sASYNCIFY=1                        # Support async operations in the compiled code (used for game loop)
        -sASYNCIFY_IGNORE_INDIRECT=1        # Assume indirect calls can't lead to an unwind/rewind of the stack (faster)
    )

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_JSPI
        -sJSPI=1                            # Use VM support for the JavaScript Promise Integration proposal
    )

    set(ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS
        $<$<CONFIG:Debug>:${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_DEBUG}>
        $<$<CONFIG:Release>:${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELEASE}>
        $<$<CONFIG:RelWithDebInfo>:${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELWITHDEBINFO}>

        ${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_ASYNCIFY}
        # ${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS_JSPI}

        -pthread                            # Enable threading support
        -sPTHREAD_POOL_SIZE_STRICT=0        # Allow thread pool to grow indefinitely

        # Raise `Error.stackTraceLimit` so Zancle's `printStackTrace` shows full chains
        --pre-js=${CMAKE_SOURCE_DIR}/emscripten/pre.js

        -Wno-limited-postlink-optimizations # warning: running limited binaryen optimizations because DWARF info requested (or indirectly required)
        -Wno-pthreads-mem-growth            # warning: -pthread + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271

        -sALLOW_MEMORY_GROWTH=1             # Grow the memory arrays at runtime
        -sEXIT_RUNTIME=1                    # Execute cleanup (e.g. `atexit`) after `main` completes
        -sFETCH=1                           # Enables `emscripten_fetch` API
        -sFORCE_FILESYSTEM=1                # Makes full filesystem support be included
        -sGL_EXPLICIT_UNIFORM_LOCATION=1    # Enables support for the `EMSCRIPTEN_explicit_uniform_location` WebGL extension
        -sMAX_WEBGL_VERSION=2               # Specifies the highest WebGL version to target
        -sMIN_WEBGL_VERSION=2               # Specifies the lowest WebGL version to target
        -sSTACK_SIZE=4mb                    # Set the total stack size
        -sTOTAL_MEMORY=128mb                # Set the total memory size (needed for `bubble_idle` example)
        -sUSE_PTHREADS=1                    # Enable threading support
        -sWASM=1                            # Compile code to WebAssembly

        # Expose low-level memory access functions to JavaScript
        -sEXPORTED_RUNTIME_METHODS=HEAPU32,HEAP8,getValue,setValue,requestFullscreen

        --emrun                             # Add native support for `emrun` (I/O capture)

        --shell-file=${CMAKE_SOURCE_DIR}/emscripten/shell.html
    )

elseif(LINUX)
    set(ZA_OS_LINUX 1)
    set(ZA_OS_UNIX 1)
elseif(BSD)
    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
        set(ZA_OS_FREEBSD 1)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
        set(ZA_OS_OPENBSD 1)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        set(ZA_OS_NETBSD 1)
    else()
        message(FATAL_ERROR "Unsupported BSD variant: ${CMAKE_SYSTEM_NAME}")
    endif()
elseif(CYGWIN)
    message(FATAL_ERROR "Unfortunately Zancle doesn't support Cygwin's 'hybrid' status between both Windows and Linux derivatives.\nIf you insist on using the GCC, please use a standalone build of MinGW without the Cygwin environment instead.")
else()
    message(FATAL_ERROR "Unsupported operating system or environment: ${CMAKE_SYSTEM_NAME}")
endif()

# Default for the ZA_OPENGL_ES build option (the option itself is declared in
# the top-level CMakeLists.txt under ZA_BUILD_WINDOW so the user can still
# override it via -DZA_OPENGL_ES=ON/OFF on the cmake command line).
if(NOT DEFINED ZA_OPENGL_ES_DEFAULT)
    if(ZA_OS_ANDROID OR ZA_OS_IOS OR ZA_OS_EMSCRIPTEN)
        set(ZA_OPENGL_ES_DEFAULT ON)
    else()
        set(ZA_OPENGL_ES_DEFAULT OFF)
    endif()
endif()

# detect the compiler
# Note: The detection is order is important because:
# - Visual Studio can both use MSVC and Clang
# - GNUCXX can still be set on macOS when using Clang
if(MSVC)
    set(ZA_COMPILER_MSVC 1)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(ZA_COMPILER_CLANG_CL 1)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(ZA_COMPILER_CLANG 1)

    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-v" OUTPUT_VARIABLE CLANG_COMPILER_VERSION ERROR_VARIABLE CLANG_COMPILER_VERSION)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(ZA_COMPILER_GCC 1)

    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-v" OUTPUT_VARIABLE GCC_COMPILER_VERSION ERROR_VARIABLE GCC_COMPILER_VERSION)
    string(REGEX MATCHALL ".*(tdm[64]*-[1-9]).*" ZA_COMPILER_GCC_TDM "${GCC_COMPILER_VERSION}")
else()
    message(WARNING "Unrecognized compiler: ${CMAKE_CXX_COMPILER_ID}. Use at your own risk.")
endif()
