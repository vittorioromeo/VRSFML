# all source files
set(SRC Sockets.cpp
        TCP.cpp
        UDP.cpp)

# define the sockets target
zancle_add_example(sockets
                 SOURCES ${SRC}
                 DEPENDS Zancle::Network)
