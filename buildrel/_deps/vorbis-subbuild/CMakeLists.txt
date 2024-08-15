# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.30.1)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(vorbis-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[C:/msys64/usr/bin/git.exe]==])
set(GIT_VERSION_STRING [==[2.46.0]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[C:/msys64/usr/bin/git.exe;2.46.0]==]
)


include(ExternalProject)
ExternalProject_Add(vorbis-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/xiph/vorbis.git" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "GIT_TAG" "v1.3.7" "GIT_SHALLOW" "ON" "PATCH_COMMAND" "C:/msys64/ucrt64/bin/cmake.exe" "-DVORBIS_DIR=C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src" "-P" "C:/OHWorkspace/SFML/src/SFML/Audio/../../../tools/vorbis/PatchVorbis.cmake"
                    SOURCE_DIR          "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src"
                    BINARY_DIR          "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


