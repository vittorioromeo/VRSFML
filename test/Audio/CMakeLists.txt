file(GLOB AUDIO_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-audio
    SOURCES ${AUDIO_SRC}
    DEPENDS Zancle::Audio
    RESOURCES
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
        killdeer.wav)
