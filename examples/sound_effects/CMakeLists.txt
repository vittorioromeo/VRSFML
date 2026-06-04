# all source files
set(SRC SoundEffects.cpp)
if(ZA_OS_IOS)
    set(RESOURCES
        resources/doodle_pop.ogg
        resources/text-background.png
        resources/tuffy.ttf)
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# define the sound_effects target
zancle_add_example(sound_effects GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS Zancle::Audio Zancle::Graphics)
