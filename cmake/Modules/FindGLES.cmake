#
# Try to find GLES library and include path.
# Once done this will define
#
# GLES_FOUND
# GLES_INCLUDE_PATH
# GLES_LIBRARY
#

find_path(GLES_INCLUDE_DIR GLES3/gl3.h)

if (SFML_OS_EMSCRIPTEN)
    set(GLES_LIBRARY GL)
else()
    find_library(GLES_LIBRARY NAMES GLESv2)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLES DEFAULT_MSG GLES_LIBRARY GLES_INCLUDE_DIR)

if (SFML_OS_EMSCRIPTEN)
    add_library(GLES::GLES INTERFACE IMPORTED)
    target_link_libraries(GLES::GLES INTERFACE ${GLES_LIBRARY})
else()
    add_library(GLES::GLES IMPORTED UNKNOWN)
    set_target_properties(GLES::GLES PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLES_INCLUDE_DIR}
        IMPORTED_LOCATION ${GLES_LIBRARY})
endif()
