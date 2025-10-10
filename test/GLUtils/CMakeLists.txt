file(GLOB GLUTILS_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-glutils "${GLUTILS_SRC}" SFML::GLUtils) # TODO P1: cleanup these circular deps
