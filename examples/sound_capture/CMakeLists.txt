# all source files
set(SRC SoundCapture.cpp)

# define the sound_capture target
sfml_add_example(sound_capture
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio)
