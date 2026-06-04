add_subdirectory(install)
set_target_warnings(test-zancle-install)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

set(TEST_SRCROOT ${PROJECT_SOURCE_DIR}/test)
file(GLOB TEST_UTILITIES_SRC CONFIGURE_DEPENDS
    "${TEST_SRCROOT}/TestUtilities/*.cpp"
    "${TEST_SRCROOT}/TestUtilities/*.hpp"
    "${TEST_SRCROOT}/TestUtilities/*.inl"
    "${TEST_SRCROOT}/TestUtilities/Tst/*.cpp"
    "${TEST_SRCROOT}/TestUtilities/Tst/*.hpp"
    "${TEST_SRCROOT}/TestUtilities/Tst/Detail/*.hpp"
)

add_library(zancle-test-main STATIC ${TEST_UTILITIES_SRC})
target_include_directories(zancle-test-main PUBLIC TestUtilities)

set_target_warnings(zancle-test-main)

target_link_libraries(zancle-test-main PUBLIC Zancle::System)

# enable precompiled headers
if (ZA_ENABLE_PCH)
    message(VERBOSE "enabling PCH for zancle-test-main")

    target_precompile_headers(zancle-test-main REUSE_FROM zancle-system)

    find_package(Threads REQUIRED)
    target_link_libraries(zancle-test-main PRIVATE Threads::Threads)
endif()

# set the target flags to use the appropriate C++ standard library
zancle_set_stdlib(zancle-test-main)

zancle_set_option(ZA_RUN_DISPLAY_TESTS ON BOOL "ON to run tests that require a display, OFF to ignore it")
if(ZA_RUN_DISPLAY_TESTS)
    add_definitions(-DZA_RUN_DISPLAY_TESTS)
    target_compile_definitions(zancle-test-main PRIVATE ZA_RUN_DISPLAY_TESTS)
endif()

zancle_set_option(ZA_RUN_AUDIO_DEVICE_TESTS ON BOOL "ON to run tests that require an audio device, OFF to ignore it")
if(ZA_RUN_AUDIO_DEVICE_TESTS)
    add_definitions(-DZA_RUN_AUDIO_DEVICE_TESTS)
    target_compile_definitions(zancle-test-main PRIVATE ZA_RUN_AUDIO_DEVICE_TESTS)
endif()

if(NOT ZA_OS_IOS)
    add_definitions(-DZA_RUN_CONNECTION_TESTS -DZA_RUN_LOOPBACK_TESTS)
    target_compile_definitions(zancle-test-main PRIVATE ZA_RUN_CONNECTION_TESTS)
    target_compile_definitions(zancle-test-main PRIVATE ZA_RUN_LOOPBACK_TESTS)
endif()

add_subdirectory(Audio)
add_subdirectory(Base)
add_subdirectory(GLUtils)
add_subdirectory(Graphics)
add_subdirectory(Network)
add_subdirectory(System)
add_subdirectory(Window)


if(ZA_OS_ANDROID AND DEFINED ENV{LIBCXX_SHARED_SO})
    # Because we can only write to the tmp directory on the Android virtual device we will need to build our directory tree under it
    set(TARGET_DIR "/data/local/tmp/$<TARGET_FILE_DIR:test-zancle-system>")

    # Generate script that copies necessary files over to the Android virtual device
    file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/prepare-android-files.sh" CONTENT
        "#!/bin/bash\n\
        adb shell \"mkdir -p ${TARGET_DIR}\"\n\
        adb push $<TARGET_FILE:test-zancle-audio>       ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-base>        ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-graphics>    ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-network>     ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-system>      ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-glutils>     ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:test-zancle-window>      ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::Audio>           ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::Graphics>        ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::ImGui>           ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::Network>         ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::System>          ${TARGET_DIR}\n\
        adb push $<TARGET_FILE:Zancle::Window>          ${TARGET_DIR}\n\
        adb push $ENV{LIBCXX_SHARED_SO}               ${TARGET_DIR}\n\
        adb push ${CMAKE_CURRENT_LIST_DIR}/Audio/.    ${TARGET_DIR}\n\
        adb push ${CMAKE_CURRENT_LIST_DIR}/Graphics/. ${TARGET_DIR}\n\
        adb push ${CMAKE_CURRENT_LIST_DIR}/System/.   ${TARGET_DIR}\n\
        adb push ${CMAKE_CURRENT_LIST_DIR}/Window/.   ${TARGET_DIR}\n\
        adb shell \"chmod -R 775 ${TARGET_DIR} && ls -la ${TARGET_DIR}\"\n"
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

    # Add the target to invoke the file copy script
    add_custom_target(prepare-android-files COMMAND "${PROJECT_BINARY_DIR}/prepare-android-files.sh")

    # Generate proxy script that translates CTest commands into adb shell commands
    file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/run-in-adb-shell.sh" CONTENT
        "#!/bin/bash\n\
        adb shell \"cd ${TARGET_DIR}; LD_LIBRARY_PATH=${TARGET_DIR} /data/local/tmp/$1 \\\"$2\\\" \\\"$3\\\" \\\"$4\\\" \\\"$5\\\" \\\"$6\\\" \\\"$7\\\" \\\"$8\\\" \\\"$9\\\"\"\n"
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
endif()

if(ZA_ENABLE_COVERAGE AND ZA_OS_WINDOWS AND NOT ZA_COMPILER_GCC)
    # Try to find and use OpenCppCoverage for coverage reporting when building with MSVC
    find_program(OpenCppCoverage_BINARY "OpenCppCoverage.exe")

    if(OpenCppCoverage_BINARY)
        execute_process(COMMAND "${OpenCppCoverage_BINARY}" --help ERROR_VARIABLE OpenCppCoverage_HELP_OUTPUT OUTPUT_QUIET)

        if(OpenCppCoverage_HELP_OUTPUT MATCHES "OpenCppCoverage Version: ([.0-9]+)")
            set(OpenCppCoverage_VERSION "${CMAKE_MATCH_1}")
        endif()
    endif()

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(OpenCppCoverage
        REQUIRED_VARS OpenCppCoverage_BINARY
        VERSION_VAR OpenCppCoverage_VERSION
    )
endif()

if(ZA_ENABLE_COVERAGE AND OpenCppCoverage_FOUND)
    # Use OpenCppCoverage
    message(STATUS "Using OpenCppCoverage to generate coverage report")

    string(REPLACE "/" "\\" COVERAGE_EXCLUDE "${CMAKE_CTEST_COMMAND}")
    string(REPLACE "/" "\\" COVERAGE_SRC "${PROJECT_SOURCE_DIR}/src")
    string(REPLACE "/" "\\" COVERAGE_INCLUDE "${PROJECT_SOURCE_DIR}/include")

    # We need to patch the OpenCppCoverage output to remove path prefixes so Coveralls doesn't get confused
    cmake_path(GET PROJECT_SOURCE_DIR ROOT_NAME COVERAGE_ROOT_NAME)
    string(REPLACE "/" "\\\\" COVERAGE_PATH_PREFIX "${PROJECT_SOURCE_DIR}/")
    string(REPLACE "${COVERAGE_ROOT_NAME}\\\\" "" COVERAGE_PATH_PREFIX "${COVERAGE_PATH_PREFIX}")

    file(WRITE "${PROJECT_BINARY_DIR}/patch_coverage.cmake"
         "file(READ \"${PROJECT_BINARY_DIR}/coverage.out\" COVERAGE_OUT)\n\
         string(REPLACE \"${COVERAGE_PATH_PREFIX}\" \"\" COVERAGE_OUT \"\${COVERAGE_OUT}\")\n\
         string(REPLACE \"${COVERAGE_ROOT_NAME}\" \".\" COVERAGE_OUT \"\${COVERAGE_OUT}\")\n\
         file(WRITE \"${PROJECT_BINARY_DIR}/coverage.out\" \"\${COVERAGE_OUT}\")\n")

    set(COVERAGE_PREFIX ${OpenCppCoverage_BINARY} --quiet --export_type cobertura:${PROJECT_BINARY_DIR}/coverage.out --cover_children --excluded_modules "${COVERAGE_EXCLUDE}" --sources "${COVERAGE_SRC}" --sources "${COVERAGE_INCLUDE}" --)
else()
    # On all other systems, we just run an empty script
    file(WRITE "${PROJECT_BINARY_DIR}/patch_coverage.cmake" "")
endif()

# Convenience for building and running tests in a single command
include(ProcessorCount)
ProcessorCount(ZA_RUNTESTS_PARALLEL)
if(ZA_RUNTESTS_PARALLEL EQUAL 0)
    set(ZA_RUNTESTS_PARALLEL 1)
endif()

add_custom_target(runtests DEPENDS test-zancle-base test-zancle-system test-zancle-glutils test-zancle-window test-zancle-graphics test-zancle-network test-zancle-audio)

add_custom_command(TARGET runtests
                   COMMENT "Run tests"
                   POST_BUILD
                   COMMAND ${COVERAGE_PREFIX} ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${ZA_RUNTESTS_PARALLEL} -C $<CONFIG>
                   COMMAND ${CMAKE_COMMAND} -P "${PROJECT_BINARY_DIR}/patch_coverage.cmake"
                   VERBATIM)
