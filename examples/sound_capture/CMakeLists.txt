# all source files
set(SRC SoundCapture.cpp)

# define the sound_capture target
zancle_add_example(sound_capture
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio)
