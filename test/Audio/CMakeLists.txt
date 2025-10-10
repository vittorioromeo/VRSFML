file(GLOB AUDIO_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-audio "${AUDIO_SRC}" SFML::Audio)

if(SFML_OS_IOS)
    set(AUDIO_RESOURCE
        ding-🐌.flac
        ding-🐌.mp3
        ding-ń.flac
        ding-ń.mp3
        ding.flac
        ding.mp3
        doodle_pop-🐌.ogg
        doodle_pop-ń.ogg
        doodle_pop.ogg
        killdeer-🐌.wav
        killdeer-ń.wav
        killdeer.wav
    )
    target_sources(test-sfml-audio PRIVATE ${AUDIO_RESOURCE})
    set_source_files_properties(${AUDIO_RESOURCE} PROPERTIES MACOSX_PACKAGE_LOCATION "Audio")
endif()
