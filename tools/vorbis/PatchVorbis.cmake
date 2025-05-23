file(READ "CMakeLists.txt" VORBIS_CMAKELISTS_CONTENTS)
string(REPLACE "cmake_minimum_required(VERSION 2.8.12)" "cmake_minimum_required(VERSION 2.8.12...3.10)" VORBIS_CMAKELISTS_CONTENTS "${VORBIS_CMAKELISTS_CONTENTS}")
string(REPLACE "find_package(Ogg REQUIRED)" "set(OGG_LIBRARIES Ogg::ogg)" VORBIS_CMAKELISTS_CONTENTS "${VORBIS_CMAKELISTS_CONTENTS}")
string(REPLACE "\n\nadd_subdirectory(lib)" "\nset(CMAKE_DEBUG_POSTFIX d)\nadd_subdirectory(lib)" VORBIS_CMAKELISTS_CONTENTS "${VORBIS_CMAKELISTS_CONTENTS}")
string(REPLACE "install(\n    FILES\n    \${CMAKE_CURRENT_BINARY_DIR}/vorbis.pc\n    \${CMAKE_CURRENT_BINARY_DIR}/vorbisenc.pc\n    \${CMAKE_CURRENT_BINARY_DIR}/vorbisfile.pc\n    DESTINATION\n    \${CMAKE_INSTALL_LIBDIR}/pkgconfig\n)" "" VORBIS_CMAKELISTS_CONTENTS "${VORBIS_CMAKELISTS_CONTENTS}")
file(WRITE "CMakeLists.txt" "${VORBIS_CMAKELISTS_CONTENTS}")

file(READ "./lib/CMakeLists.txt" VORBISLIB_CMAKELISTS_CONTENTS)
string(REPLACE "install(FILES \${VORBIS_PUBLIC_HEADERS} DESTINATION \${CMAKE_INSTALL_FULL_INCLUDEDIR}/vorbis)" "" VORBISLIB_CMAKELISTS_CONTENTS "${VORBISLIB_CMAKELISTS_CONTENTS}")
file(WRITE "./lib/CMakeLists.txt" "${VORBISLIB_CMAKELISTS_CONTENTS}")
