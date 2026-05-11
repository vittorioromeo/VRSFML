file(GLOB EXAMPLE_UTILS_TEST_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-example-utils "${EXAMPLE_UTILS_TEST_SRC}" ExampleUtils)
target_include_directories(test-example-utils PRIVATE ${PROJECT_SOURCE_DIR}/examples/include)
