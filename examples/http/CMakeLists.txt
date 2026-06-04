# all source files
set(SRC Http.cpp)

# define the http target
zancle_add_example(http
                 SOURCES ${SRC}
                 DEPENDS Zancle::Network)
