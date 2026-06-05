file(GLOB SYSTEM_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-system
    SOURCES ${SYSTEM_SRC}
    RESOURCES
        test.txt
        test2.txt
        test-ń.txt
        test-🐌.txt)

target_compile_definitions(test-zancle-system PRIVATE
    EXPECTED_ZA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    EXPECTED_ZA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    EXPECTED_ZA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    EXPECTED_ZA_VERSION_IS_RELEASE=$<IF:$<BOOL:${VERSION_IS_RELEASE}>,true,false>
)
