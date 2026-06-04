file(GLOB AUDIO_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-audio "${AUDIO_SRC}" Zancle::Audio)

if(ZA_OS_IOS OR ZA_OS_EMSCRIPTEN)
    set(AUDIO_RESOURCES
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

    target_sources(test-zancle-audio PRIVATE ${AUDIO_RESOURCES})
    set_source_files_properties(${AUDIO_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Audio")

    if(ZA_OS_EMSCRIPTEN)
        # Use --preload-file instead of --embed-file to support non-ASCII filenames.
        # --embed-file generates assembly symbols derived from filenames, which the
        # assembler rejects for non-ASCII characters (e.g. ń, 🐌). --preload-file
        # stores filenames as JS strings in a .data bundle, avoiding the issue.
        set(AUDIO_RESOURCE_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/staging_test-zancle-audio")

        foreach(RESOURCE ${AUDIO_RESOURCES})
            file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE}" DESTINATION "${AUDIO_RESOURCE_STAGING_DIR}")
        endforeach()

        target_link_options(test-zancle-audio PRIVATE
            "SHELL:--preload-file ${AUDIO_RESOURCE_STAGING_DIR}@/")
    endif()
endif()
