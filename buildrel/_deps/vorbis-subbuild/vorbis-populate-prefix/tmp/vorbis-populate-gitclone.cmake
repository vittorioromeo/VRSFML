# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitclone-lastrun.txt" AND EXISTS "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitinfo.txt" AND
  "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitclone-lastrun.txt" IS_NEWER_THAN "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitinfo.txt")
  message(VERBOSE
    "Avoiding repeated git clone, stamp file is up to date: "
    "'C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitclone-lastrun.txt'"
  )
  return()
endif()

# Even at VERBOSE level, we don't want to see the commands executed, but
# enabling them to be shown for DEBUG may be useful to help diagnose problems.
cmake_language(GET_MESSAGE_LOG_LEVEL active_log_level)
if(active_log_level MATCHES "DEBUG|TRACE")
  set(maybe_show_command COMMAND_ECHO STDOUT)
else()
  set(maybe_show_command "")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: 'C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "C:/msys64/usr/bin/git.exe"
            clone --no-checkout --depth 1 --no-single-branch --config "advice.detachedHead=false" "https://github.com/xiph/vorbis.git" "vorbis-src"
    WORKING_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(NOTICE "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/xiph/vorbis.git'")
endif()

execute_process(
  COMMAND "C:/msys64/usr/bin/git.exe"
          checkout "v1.3.7" --
  WORKING_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v1.3.7'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "C:/msys64/usr/bin/git.exe" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: 'C:/OHWorkspace/SFML/buildrel/_deps/vorbis-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitinfo.txt" "C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: 'C:/OHWorkspace/SFML/buildrel/_deps/vorbis-subbuild/vorbis-populate-prefix/src/vorbis-populate-stamp/vorbis-populate-gitclone-lastrun.txt'")
endif()
