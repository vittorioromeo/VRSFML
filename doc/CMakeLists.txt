# find doxygen
if(ZA_OS_MACOS)
    # Add some path to search doxygen in more directories.
    set(ADDITIONAL_PATHS
        /Developer/Applications/Doxygen.app/Contents/Resources
        /Developer/Applications/Doxygen.app/Contents/MacOS
        $ENV{HOME}/Applications/Doxygen.app/Contents/Resources
        $ENV{HOME}/Applications/Doxygen.app/Contents/MacOS
        $ENV{HOME}/Applications/Developer/Doxygen.app/Contents/Resources
        $ENV{HOME}/Applications/Developer/Doxygen.app/Contents/MacOS)

    list(APPEND CMAKE_PROGRAM_PATH ${ADDITIONAL_PATHS})
endif()

find_package(Doxygen REQUIRED)

# doxygen-awesome-css: a CSS+JS theme for Doxygen that gives the
# generated HTML a modern look (dark mode toggle, sticky sidebar,
# copy-to-clipboard buttons, paragraph anchors, interactive TOC).
# The Doxygen comment syntax is untouched -- this is pure presentation.
CPMAddPackage(
    NAME doxygen-awesome-css
    GITHUB_REPOSITORY jothepro/doxygen-awesome-css
    GIT_TAG v2.4.2
    DOWNLOAD_ONLY TRUE
)

set(DOXYGEN_AWESOME_DIR "${doxygen-awesome-css_SOURCE_DIR}")

# configure the source Doxyfile by copying it and replacing all @variables@
configure_file(doxyfile.in doxyfile @ONLY)
configure_file(header.html.in header.html @ONLY)

# target setup
add_custom_target(doc
                  COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
                  COMMAND Doxygen::doxygen ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
                  COMMAND ${CMAKE_COMMAND} -E echo "Done."
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# setup install rules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Zancle.tag
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT doc)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT doc)
