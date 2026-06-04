# all source files
set(SRC Tennis.cpp)

if(ZA_OS_IOS)
    set(RESOURCES
        resources/ball.wav
        resources/zancle_logo.png
        resources/tuffy.ttf)
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# define the tennis target
zancle_add_example(tennis GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS Zancle::Audio Zancle::Graphics)
