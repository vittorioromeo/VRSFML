file(GLOB NETWORK_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-network "${NETWORK_SRC}" SFML::Network)
