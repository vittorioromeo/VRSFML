file(GLOB NETWORK_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-network "${NETWORK_SRC}" Zancle::Network)
