file(GLOB WINDOW_SRC CONFIGURE_DEPENDS "${TEST_SRCROOT}/Window/*.cpp" "${TEST_SRCROOT}/Window/*.hpp" "${TEST_SRCROOT}/Window/*.inl")
sfml_add_test(test-sfml-window "${WINDOW_SRC}" SFML::Window) # TODO P1: cleanup these circular deps
