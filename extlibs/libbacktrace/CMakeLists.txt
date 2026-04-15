# Vendored libbacktrace (https://github.com/ianlancetaylor/libbacktrace)
# License: see src/LICENSE (3-clause BSD)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckTypeSize)

set(LBT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LBT_INC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --------------------------------------------------------------------
# Header / function probes
# --------------------------------------------------------------------
check_include_file(dlfcn.h        HAVE_DLFCN_H)
check_include_file(inttypes.h     HAVE_INTTYPES_H)
check_include_file(link.h         HAVE_LINK_H)
check_include_file(mach-o/dyld.h  HAVE_MACH_O_DYLD_H)
check_include_file(memory.h       HAVE_MEMORY_H)
check_include_file(stdint.h       HAVE_STDINT_H)
check_include_file(stdlib.h       HAVE_STDLIB_H)
check_include_file(strings.h      HAVE_STRINGS_H)
check_include_file(string.h       HAVE_STRING_H)
check_include_file(sys/ldr.h      HAVE_SYS_LDR_H)
check_include_file(sys/link.h     HAVE_SYS_LINK_H)
check_include_file(sys/mman.h     HAVE_SYS_MMAN_H)
check_include_file(sys/stat.h     HAVE_SYS_STAT_H)
check_include_file(sys/types.h    HAVE_SYS_TYPES_H)
check_include_file(tlhelp32.h     HAVE_TLHELP32_H)
check_include_file(unistd.h       HAVE_UNISTD_H)
check_include_file(windows.h      HAVE_WINDOWS_H)

check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
check_function_exists(fcntl         HAVE_FCNTL)
check_function_exists(lstat         HAVE_LSTAT)
check_function_exists(readlink      HAVE_READLINK)
check_function_exists(getexecname   HAVE_GETEXECNAME)

check_symbol_exists(getpagesize  unistd.h  HAVE_DECL_GETPAGESIZE)
check_symbol_exists(strnlen      string.h  HAVE_DECL_STRNLEN)
check_symbol_exists(_pgmptr      stdlib.h  HAVE_DECL__PGMPTR)

# dl_iterate_phdr (glibc / musl / BSD)
check_c_source_compiles("
    #define _GNU_SOURCE
    #include <link.h>
    static int cb(struct dl_phdr_info* i, size_t s, void* d){(void)i;(void)s;(void)d;return 0;}
    int main(void){dl_iterate_phdr(cb, 0);return 0;}
" HAVE_DL_ITERATE_PHDR)

# __atomic_* intrinsics
check_c_source_compiles("
    int main(void){int x=0,y=1;__atomic_load_n(&x,__ATOMIC_ACQUIRE);
    __atomic_store_n(&x,y,__ATOMIC_RELEASE);return 0;}
" HAVE_ATOMIC_FUNCTIONS)

# __sync_* intrinsics (legacy)
check_c_source_compiles("
    int main(void){int x=0;__sync_bool_compare_and_swap(&x,0,1);
    __sync_synchronize();return 0;}
" HAVE_SYNC_FUNCTIONS)

# _Unwind_GetIPInfo (needs libgcc_s)
check_c_source_compiles("
    #include <unwind.h>
    int main(void){int b;_Unwind_GetIPInfo(0,&b);return 0;}
" HAVE_GETIPINFO)

# sysctl-based executable path (BSD/macOS)
check_c_source_compiles("
    #include <sys/types.h>
    #include <sys/sysctl.h>
    int main(void){int m[4]={CTL_KERN,KERN_PROC,KERN_PROC_PATHNAME,-1};return m[0];}
" HAVE_KERN_PROC)

check_c_source_compiles("
    #include <sys/types.h>
    #include <sys/sysctl.h>
    int main(void){int m[4]={CTL_KERN,KERN_PROCARGS,KERN_PROC_PATHNAME,-1};return m[0];}
" HAVE_KERN_PROC_ARGS)

# No optional compression
set(HAVE_LIBLZMA 0)
set(HAVE_ZLIB    0)
set(HAVE_ZSTD    0)
set(HAVE_LOADQUERY 0)

# --------------------------------------------------------------------
# Platform-dependent source selection
# --------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux"
    OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
    OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD"
    OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD"
    OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(LBT_FORMAT_FILE elf.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LBT_FORMAT_FILE macho.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LBT_FORMAT_FILE pecoff.c)
else()
    set(LBT_FORMAT_FILE unknown.c)
endif()

# ELF size
if(LBT_FORMAT_FILE STREQUAL "elf.c")
    check_type_size("void*" LBT_SIZEOF_VOIDP BUILTIN_TYPES_ONLY)
    if(LBT_SIZEOF_VOIDP EQUAL 8)
        set(BACKTRACE_ELF_SIZE 64)
    else()
        set(BACKTRACE_ELF_SIZE 32)
    endif()
endif()

# View / alloc backend: prefer mmap on POSIX, malloc-based on Windows
if(HAVE_SYS_MMAN_H AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LBT_VIEW_FILE  mmapio.c)
    set(LBT_ALLOC_FILE mmap.c)
    set(BACKTRACE_USES_MALLOC 0)
else()
    set(LBT_VIEW_FILE  read.c)
    set(LBT_ALLOC_FILE alloc.c)
    set(BACKTRACE_USES_MALLOC 1)
endif()

# Supported iff we have an unwind backend + a format backend
if(NOT LBT_FORMAT_FILE STREQUAL "unknown.c" AND HAVE_GETIPINFO)
    set(BACKTRACE_SUPPORTED 1)
else()
    set(BACKTRACE_SUPPORTED 0)
endif()

set(BACKTRACE_SUPPORTS_THREADS 1)
set(BACKTRACE_SUPPORTS_DATA 1)

# --------------------------------------------------------------------
# Generate headers
# --------------------------------------------------------------------
set(LBT_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${LBT_GEN_DIR})

configure_file(${LBT_SRC}/config.h.cmake.in               ${LBT_GEN_DIR}/config.h)
configure_file(${LBT_SRC}/backtrace-supported.h.cmake.in  ${LBT_GEN_DIR}/backtrace-supported.h)

# --------------------------------------------------------------------
# Target
# --------------------------------------------------------------------
set(LBT_SOURCES
    ${LBT_SRC}/atomic.c
    ${LBT_SRC}/backtrace.c
    ${LBT_SRC}/dwarf.c
    ${LBT_SRC}/fileline.c
    ${LBT_SRC}/posix.c
    ${LBT_SRC}/print.c
    ${LBT_SRC}/sort.c
    ${LBT_SRC}/state.c
    ${LBT_SRC}/${LBT_FORMAT_FILE}
    ${LBT_SRC}/${LBT_VIEW_FILE}
    ${LBT_SRC}/${LBT_ALLOC_FILE}
)

add_library(sfml-libbacktrace STATIC ${LBT_SOURCES})
add_library(sfml::libbacktrace ALIAS sfml-libbacktrace)

target_include_directories(sfml-libbacktrace
    PUBLIC  ${LBT_INC}
    PRIVATE ${LBT_SRC} ${LBT_GEN_DIR}
)

set_target_properties(sfml-libbacktrace PROPERTIES
    C_STANDARD 11
    POSITION_INDEPENDENT_CODE ON
    UNITY_BUILD OFF
)

# Silence libbacktrace's own warnings; upstream uses -Wall -Wextra and we
# don't want to fight it when their code trips our strict flags.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(sfml-libbacktrace PRIVATE -w)
endif()
