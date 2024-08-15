# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/OHWorkspace/SFML/buildrel/_deps/doctest-src")
  file(MAKE_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps/doctest-src")
endif()
file(MAKE_DIRECTORY
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-build"
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix"
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/tmp"
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp"
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/src"
  "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
