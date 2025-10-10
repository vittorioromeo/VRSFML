file(GLOB WINDOW_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-window "${WINDOW_SRC}" SFML::Window) # TODO P1: cleanup these circular deps
