file(GLOB BASE_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-base "${BASE_SRC}" "")
