# all source files
set(SRC SoundMultiDevice.cpp)

# define the sound target
sfml_add_example(sound-multi-device
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio
                 RESOURCES_DIR resources)
