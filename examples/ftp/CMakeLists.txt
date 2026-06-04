# all source files
set(SRC Ftp.cpp)

# define the ftp target
zancle_add_example(ftp
                 SOURCES ${SRC}
                 DEPENDS Zancle::Network)
