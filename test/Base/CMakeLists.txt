file(GLOB BASE_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-base SOURCES ${BASE_SRC})
