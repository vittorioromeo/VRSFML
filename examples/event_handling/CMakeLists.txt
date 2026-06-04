# all source files
set(SRC EventHandling.cpp)

if(ZA_OS_IOS)
    set(RESOURCES
        resources/tuffy.ttf)
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# define the event_handling target
zancle_add_example(event_handling GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS Zancle::Graphics)
