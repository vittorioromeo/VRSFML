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
                 SOURCES ${SRC})

# setup dependencies
target_link_libraries(sfml-network PUBLIC SFML::System)
if(SFML_OS_WINDOWS)
    target_link_libraries(sfml-network PRIVATE ws2_32)
endif()
