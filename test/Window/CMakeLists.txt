file(GLOB WINDOW_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-window "${WINDOW_SRC}" Zancle::Window) # TODO P1: cleanup these circular deps
