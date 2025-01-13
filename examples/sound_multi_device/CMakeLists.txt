# all source files
set(SRC SoundMultiDevice.cpp)

# define the sound_multi_device target
sfml_add_example(sound_multi_device
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio)
