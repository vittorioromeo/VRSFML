# all source files
set(SRC CoroutineTutorial.cpp)

if(ZA_OS_IOS)
    set(RESOURCES
        resources/tuffy.ttf)
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# define the coroutine_tutorial target
zancle_add_example(coroutine_tutorial GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS Zancle::Graphics)
