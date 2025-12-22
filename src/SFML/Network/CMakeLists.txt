set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Network)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# add platform specific sources
if(SFML_OS_WINDOWS)
    list(APPEND SRC
        ${SRCROOT}/Win32/SocketImpl.cpp
    )
else()
    list(APPEND SRC
        ${SRCROOT}/Unix/SocketImpl.cpp
    )
endif()

# define the sfml-network target
sfml_add_library(Network
                 SOURCES ${SRC}
                 DEPENDENCIES "Dependencies.cmake.in")

# setup dependencies
target_link_libraries(sfml-network PUBLIC SFML::System)
if(SFML_OS_WINDOWS)
    target_link_libraries(sfml-network PRIVATE Crypt32 ws2_32)
endif()

# use a block to scope option variables we have to set
block()
    include(FetchContent)

    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS OFF)
    set(ENABLE_TESTING OFF)
    set(MBEDTLS_AS_SUBPROJECT OFF)
    set(ENABLE_PROGRAMS OFF)
    set(MBEDTLS_FATAL_WARNINGS OFF)
    set(GEN_FILES OFF)

    # TODO P0: change to use CPM
    FetchContent_Declare(MbedTLS
        URL https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.5/mbedtls-3.6.5.tar.bz2
        URL_HASH SHA256=4a11f1777bb95bf4ad96721cac945a26e04bf19f57d905f241fe77ebeddf46d8
        DOWNLOAD_NO_PROGRESS ON
        # patch out parts we don't want of the MbedTLS CMake configuration and write to configuration files
        # - disable building 3rdparty targets
        # - add CMAKE_DEBUG_POSTFIX
        # - configure mbedtls_config.h
        # - provide threading_alt.h
        PATCH_COMMAND ${CMAKE_COMMAND} -DMBEDTLS_DIR=${FETCHCONTENT_BASE_DIR}/mbedtls-src -P ${PROJECT_SOURCE_DIR}/tools/mbedtls/PatchMbedTLS.cmake)
    FetchContent_MakeAvailable(MbedTLS)
endblock()

set_target_properties(mbedtls mbedcrypto mbedx509 PROPERTIES FOLDER "Dependencies" SYSTEM ON)

# if building SFML as a shared library and linking our dependencies in
# as static libraries we need to build them with -fPIC
if(BUILD_SHARED_LIBS)
    set_target_properties(mbedtls mbedcrypto mbedx509 PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# disable building dependencies as part of a unity build, they don't support it
set_target_properties(mbedtls mbedcrypto mbedx509 PROPERTIES UNITY_BUILD OFF)

sfml_set_stdlib(mbedtls)
sfml_set_stdlib(mbedcrypto)
sfml_set_stdlib(mbedx509)

target_link_libraries(sfml-network PRIVATE mbedtls mbedcrypto mbedx509)

if(SFML_OS_MACOS)
    target_link_libraries(sfml-network PRIVATE "-framework Security" "-framework CoreFoundation")
endif()

# TODO P1: update mbedtls to 4.x
