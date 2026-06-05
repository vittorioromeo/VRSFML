file(GLOB NETWORK_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-network SOURCES ${NETWORK_SRC} DEPENDS Zancle::Network)
