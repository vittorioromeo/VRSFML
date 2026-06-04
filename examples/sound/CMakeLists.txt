# all source files
set(SRC Sound.cpp)

# define the sound target
zancle_add_example(sound
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio)
