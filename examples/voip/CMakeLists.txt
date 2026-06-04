# all source files
set(SRC VoIP.cpp
        Client.cpp
        Server.cpp)

# define the voip target
zancle_add_example(voip
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio Zancle::Network)
