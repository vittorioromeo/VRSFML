file(GLOB WINDOW_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-window SOURCES ${WINDOW_SRC} DEPENDS Zancle::Window) # TODO P1: cleanup these circular deps
